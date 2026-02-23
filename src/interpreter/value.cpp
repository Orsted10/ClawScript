#include "value.h"
#include "callable.h"
#include "array.h"
#include "features/hashmap.h"  // Added!
#include <sstream>
#include <iomanip>
#include <cmath>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <algorithm>
#include <vector>
#include "features/class.h"
#include "observability/profiler.h"
#include "vm/vm.h"

namespace claw {

// Thread-local set to track visited objects for cycle detection during string conversion
thread_local std::set<const void*> visitedObjects;

// Global registries to keep shared_ptr lifetimes while storing raw pointers in Values
static std::unordered_map<void*, std::shared_ptr<Callable>> g_callableRegistry;
static std::unordered_map<void*, std::shared_ptr<ClawArray>> g_arrayRegistry;
static std::unordered_map<void*, std::shared_ptr<ClawHashMap>> g_hashMapRegistry;
static std::unordered_map<void*, std::shared_ptr<ClawClass>> g_classRegistry;
static std::unordered_map<void*, std::shared_ptr<ClawInstance>> g_instanceRegistry;
static std::unordered_map<void*, std::shared_ptr<VMFunction>> g_vmFunctionRegistry;
static std::unordered_map<void*, std::shared_ptr<VMClosure>> g_vmClosureRegistry;
static std::unordered_map<void*, uint8_t> g_objectGeneration;
static std::unordered_set<const void*> g_rememberedSet;
static std::vector<class VM*> g_vmRegistry;
static std::atomic<uint64_t> g_youngAllocations{0};
static std::vector<std::shared_ptr<ClawArray>> g_arrayPool;
static std::vector<std::shared_ptr<ClawHashMap>> g_hashMapPool;
static thread_local std::vector<std::vector<void*>> g_ephemeralStack;
static std::atomic<bool> g_benchmarkMode{false};

static void gcMark(Value v);
static void gcMinor();
static void gcFull();
void gcRegisterVM(VM* vm) { g_vmRegistry.push_back(vm); }
void gcUnregisterVM(VM* vm) { g_vmRegistry.erase(std::remove(g_vmRegistry.begin(), g_vmRegistry.end(), vm), g_vmRegistry.end()); }
void gcBarrierWrite(const void* parent, Value child) {
    if (!isObject(child)) return;
    auto itParentGen = g_objectGeneration.find(const_cast<void*>(parent));
    if (itParentGen == g_objectGeneration.end()) return;
    auto itChildGen = g_objectGeneration.find(asObjectPtr(child));
    if (itChildGen == g_objectGeneration.end()) return;
    if (itParentGen->second == 1 && itChildGen->second == 0) {
        g_rememberedSet.insert(parent);
    }
}
void gcMaybeCollect() {
    if (g_benchmarkMode.load(std::memory_order_relaxed)) return;
    uint64_t n = ++g_youngAllocations;
    if (n % 100000 == 0) gcMinor();
    if (g_objectGeneration.size() > 1000000) gcFull();
}
static void gcMarkObject(void* p) {
    if (!p) return;
    g_objectGeneration[p] |= 0x80;
    auto arrIt = g_arrayRegistry.find(p);
    if (arrIt != g_arrayRegistry.end()) {
        const auto& elems = arrIt->second->elements();
        for (const auto& e : elems) gcMark(e);
        return;
    }
    auto mapIt = g_hashMapRegistry.find(p);
    if (mapIt != g_hashMapRegistry.end()) {
        for (const auto& kv : mapIt->second->data) {
            gcMark(kv.second);
        }
        return;
    }
    auto instIt = g_instanceRegistry.find(p);
    if (instIt != g_instanceRegistry.end()) {
        instIt->second->forEachField([](Value v){ gcMark(v); });
        return;
    }
}
static void gcMark(Value v) {
    if (isObject(v)) {
        void* p = asObjectPtr(v);
        if (!p) return;
        if ((g_objectGeneration[p] & 0x80) == 0) {
            gcMarkObject(p);
        }
    }
}
static void gcMarkVMRoots(VM* vm);
static void gcMinor() {
    for (auto vm : g_vmRegistry) gcMarkVMRoots(vm);
    for (const void* parent : g_rememberedSet) gcMarkObject(const_cast<void*>(parent));
    std::vector<void*> toFree;
    toFree.reserve(g_objectGeneration.size());
    for (auto& [p, meta] : g_objectGeneration) {
        bool marked = (meta & 0x80) != 0;
        uint8_t gen = meta & 0x1;
        if (gen == 0) {
            if (!marked) {
                toFree.push_back(p);
            } else {
                g_objectGeneration[p] = 0x81;
            }
        } else {
            g_objectGeneration[p] &= 0x81;
        }
    }
    for (void* p : toFree) {
        {
            auto it = g_arrayRegistry.find(p);
            if (it != g_arrayRegistry.end()) {
                it->second->reserve(it->second->size());
                it->second->fill(nilValue(), 0);
                g_arrayPool.push_back(std::move(it->second));
                g_arrayRegistry.erase(it);
                continue;
            }
        }
        {
            auto it = g_hashMapRegistry.find(p);
            if (it != g_hashMapRegistry.end()) {
                it->second->clear();
                g_hashMapPool.push_back(std::move(it->second));
                g_hashMapRegistry.erase(it);
                continue;
            }
        }
        if (g_instanceRegistry.erase(p)) continue;
        if (g_classRegistry.erase(p)) continue;
        if (g_callableRegistry.erase(p)) continue;
        if (g_vmFunctionRegistry.erase(p)) continue;
        if (g_vmClosureRegistry.erase(p)) continue;
        g_objectGeneration.erase(p);
    }
    g_rememberedSet.clear();
    for (auto& [p, meta] : g_objectGeneration) { meta &= 0x01; }
}
static void gcFull() {
    for (auto vm : g_vmRegistry) gcMarkVMRoots(vm);
    for (const void* parent : g_rememberedSet) gcMarkObject(const_cast<void*>(parent));
    std::vector<void*> toFree;
    toFree.reserve(g_objectGeneration.size());
    for (auto& [p, meta] : g_objectGeneration) {
        bool marked = (meta & 0x80) != 0;
        if (!marked) {
            toFree.push_back(p);
        } else {
            if ((meta & 0x1) == 0) meta = 0x01; // promote survivors
        }
    }
    for (void* p : toFree) {
        {
            auto it = g_arrayRegistry.find(p);
            if (it != g_arrayRegistry.end()) {
                it->second->reserve(it->second->size());
                it->second->fill(nilValue(), 0);
                g_arrayPool.push_back(std::move(it->second));
                g_arrayRegistry.erase(it);
                continue;
            }
        }
        {
            auto it = g_hashMapRegistry.find(p);
            if (it != g_hashMapRegistry.end()) {
                it->second->clear();
                g_hashMapPool.push_back(std::move(it->second));
                g_hashMapRegistry.erase(it);
                continue;
            }
        }
        if (g_instanceRegistry.erase(p)) continue;
        if (g_classRegistry.erase(p)) continue;
        if (g_callableRegistry.erase(p)) continue;
        if (g_vmFunctionRegistry.erase(p)) continue;
        if (g_vmClosureRegistry.erase(p)) continue;
        g_objectGeneration.erase(p);
    }
    g_rememberedSet.clear();
    for (auto& [p, meta] : g_objectGeneration) { meta &= 0x01; }
}

Value callableValue(std::shared_ptr<Callable> fn) {
    void* p = fn.get();
    g_callableRegistry[p] = std::move(fn);
    g_objectGeneration[p] = 0;
    gcMaybeCollect();
    profilerRecordAlloc(sizeof(Callable), "callable");
    return objectValue(p);
}
Value arrayValue(std::shared_ptr<ClawArray> arr) {
    void* p = arr.get();
    g_arrayRegistry[p] = std::move(arr);
    g_objectGeneration[p] = 0;
    if (!g_ephemeralStack.empty()) g_ephemeralStack.back().push_back(p);
    gcMaybeCollect();
    profilerRecordAlloc(sizeof(ClawArray), "array");
    return objectValue(p);
}
Value hashMapValue(std::shared_ptr<ClawHashMap> map) {
    void* p = map.get();
    g_hashMapRegistry[p] = std::move(map);
    g_objectGeneration[p] = 0;
    if (!g_ephemeralStack.empty()) g_ephemeralStack.back().push_back(p);
    gcMaybeCollect();
    profilerRecordAlloc(sizeof(ClawHashMap), "hashmap");
    return objectValue(p);
}
Value classValue(std::shared_ptr<ClawClass> cls) {
    void* p = cls.get();
    g_classRegistry[p] = std::move(cls);
    g_objectGeneration[p] = 0;
    gcMaybeCollect();
    profilerRecordAlloc(sizeof(ClawClass), "class");
    return objectValue(p);
}
Value instanceValue(std::shared_ptr<ClawInstance> inst) {
    void* p = inst.get();
    g_instanceRegistry[p] = std::move(inst);
    g_objectGeneration[p] = 0;
    gcMaybeCollect();
    profilerRecordAlloc(sizeof(ClawInstance), "instance");
    return objectValue(p);
}
Value vmFunctionValue(std::shared_ptr<VMFunction> fn) {
    void* p = fn.get();
    g_vmFunctionRegistry[p] = std::move(fn);
    g_objectGeneration[p] = 1;
    profilerRecordAlloc(sizeof(VMFunction), "vmfunc");
    return objectValue(p);
}
Value vmClosureValue(std::shared_ptr<VMClosure> closure) {
    void* p = closure.get();
    g_vmClosureRegistry[p] = std::move(closure);
    g_objectGeneration[p] = 1;
    return objectValue(p);
}

bool isTruthy(Value v) {
    if (isNil(v)) return false;
    if (isBool(v)) return asBool(v);
    if (isNumber(v)) return asNumber(v) != 0.0;
    if (isString(v)) return *asStringPtr(v) != '\0';
    if (isArray(v)) return asArray(v)->length() > 0;
    if (isHashMap(v)) return asHashMap(v)->size() > 0;  // Added!
    return true;
}

bool isEqual(Value a, Value b) {
    if (isNil(a) && isNil(b)) return true;
    if (isNil(a) || isNil(b)) return false;
    
    if (isNumber(a) && isNumber(b)) {
        return asNumber(a) == asNumber(b);
    }
    if (isString(a) && isString(b)) {
        return asStringPtr(a) == asStringPtr(b);
    }
    if (isBool(a) && isBool(b)) {
        return asBool(a) == asBool(b);
    }
    if (isCallable(a) && isCallable(b)) {
        return asObjectPtr(a) == asObjectPtr(b);
    }
    // Arrays compare by reference
    if (isArray(a) && isArray(b)) {
        return asArray(a) == asArray(b);
    }
    // Hash maps compare by reference  // Added!
    if (isHashMap(a) && isHashMap(b)) {
        return asHashMap(a) == asHashMap(b);
    }
    
    return false;
}

std::string valueToString(const Value& v) {
    // Call the helper function with the thread-local visited set
    return valueToStringWithCycleDetection(v, visitedObjects);
}

std::string valueToStringWithCycleDetection(const Value& v, std::set<const void*>& visited) {
    if (isNil(v)) {
        return "nil";
    } else if (isNumber(v)) {
        std::ostringstream oss;
        double num = asNumber(v);
        if (std::floor(num) == num) {
            oss << std::fixed << std::setprecision(0) << num;
            return oss.str();
        } else {
            oss << std::fixed << std::setprecision(6) << num;
            std::string str = oss.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (!str.empty() && str.back() == '.') str.pop_back();
            return str;
        }
    } else if (isString(v)) {
        return asString(v);
    } else if (isBool(v)) {
        return asBool(v) ? "true" : "false";
    } else if (isCallable(v)) {
        auto fn = asCallable(v);
        return fn ? fn->toString() : "<fn>";
    } else if (isArray(v)) {
        // For arrays, we need to also check for circular references
        auto arr = asArray(v);
        const void* ptr = arr.get();
        
        if (visited.count(ptr)) {
            return "[Circular Array]";
        }
        
        visited.insert(ptr);
        std::string result = arr->toStringWithCycleDetection(visited);
        visited.erase(ptr);
        return result;
    } else if (isHashMap(v)) {  // Added!
        auto map = asHashMap(v);
        const void* ptr = map.get();
        
        // Check if this map is already being processed (cycle detection)
        if (visited.count(ptr)) {
            return "{Circular Object}";
        }
        
        // Mark this map as being processed
        visited.insert(ptr);
        
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : map->data) {
            if (!first) oss << ", ";
            oss << "\"" << key << "\": " << valueToStringWithCycleDetection(value, visited);
            first = false;
        }
        oss << "}";
        
        // Unmark this map after processing
        visited.erase(ptr);
        return oss.str();
    }
    return "unknown";
}
bool diagnosticsEnabled() { return gRuntimeFlags.icDiagnostics; }

bool isCallable(Value v) { return isObject(v) && g_callableRegistry.count(asObjectPtr(v)) > 0; }
bool isArray(Value v) { return isObject(v) && g_arrayRegistry.count(asObjectPtr(v)) > 0; }
bool isHashMap(Value v) { return isObject(v) && g_hashMapRegistry.count(asObjectPtr(v)) > 0; }
bool isClass(Value v) { return isObject(v) && g_classRegistry.count(asObjectPtr(v)) > 0; }
bool isInstance(Value v) { return isObject(v) && g_instanceRegistry.count(asObjectPtr(v)) > 0; }
bool isVMFunction(Value v) { return isObject(v) && g_vmFunctionRegistry.count(asObjectPtr(v)) > 0; }
bool isVMClosure(Value v) { return isObject(v) && g_vmClosureRegistry.count(asObjectPtr(v)) > 0; }

std::shared_ptr<ClawArray> asArray(Value v) { auto it = g_arrayRegistry.find(asObjectPtr(v)); return it != g_arrayRegistry.end() ? it->second : nullptr; }
std::shared_ptr<ClawHashMap> asHashMap(Value v) { auto it = g_hashMapRegistry.find(asObjectPtr(v)); return it != g_hashMapRegistry.end() ? it->second : nullptr; }
std::shared_ptr<ClawClass> asClass(Value v) { auto it = g_classRegistry.find(asObjectPtr(v)); return it != g_classRegistry.end() ? it->second : nullptr; }
std::shared_ptr<ClawInstance> asInstance(Value v) { auto it = g_instanceRegistry.find(asObjectPtr(v)); return it != g_instanceRegistry.end() ? it->second : nullptr; }
std::shared_ptr<Callable> asCallable(Value v) { auto it = g_callableRegistry.find(asObjectPtr(v)); return it != g_callableRegistry.end() ? it->second : nullptr; }
std::shared_ptr<VMFunction> asVMFunction(Value v) { auto it = g_vmFunctionRegistry.find(asObjectPtr(v)); return it != g_vmFunctionRegistry.end() ? it->second : nullptr; }
std::shared_ptr<VMClosure> asVMClosure(Value v) { auto it = g_vmClosureRegistry.find(asObjectPtr(v)); return it != g_vmClosureRegistry.end() ? it->second : nullptr; }
VMClosure* asVMClosurePtr(Value v) { auto it = g_vmClosureRegistry.find(asObjectPtr(v)); return it != g_vmClosureRegistry.end() ? it->second.get() : nullptr; }

static void gcMarkVMRoots(VM* vm) {
    if (!vm) return;
    std::vector<Value> roots;
    roots.reserve(512);
    vm->forEachRoot([&](Value v) { roots.push_back(v); });
    for (auto v : roots) gcMark(v);
}

std::shared_ptr<ClawArray> gcAcquireArrayFromPool() {
    if (!g_arrayPool.empty()) {
        auto a = std::move(g_arrayPool.back());
        g_arrayPool.pop_back();
        return a;
    }
    return nullptr;
}
void gcReleaseArrayToPool(std::shared_ptr<ClawArray> arr) {
    if (arr) {
        arr->fill(nilValue(), 0);
        g_arrayPool.push_back(std::move(arr));
    }
}
std::shared_ptr<ClawHashMap> gcAcquireHashMapFromPool() {
    if (!g_hashMapPool.empty()) {
        auto m = std::move(g_hashMapPool.back());
        g_hashMapPool.pop_back();
        return m;
    }
    return nullptr;
}
void gcReleaseHashMapToPool(std::shared_ptr<ClawHashMap> map) {
    if (map) {
        map->clear();
        g_hashMapPool.push_back(std::move(map));
    }
}

void gcEphemeralFrameEnter() { g_ephemeralStack.emplace_back(); }
void gcEphemeralEscape(Value v) {
    if (g_ephemeralStack.empty()) return;
    if (!isObject(v)) return;
    void* p = asObjectPtr(v);
    auto& top = g_ephemeralStack.back();
    for (auto it = top.begin(); it != top.end(); ++it) {
        if (*it == p) { top.erase(it); break; }
    }
}
void gcEphemeralEscapeDeep(Value v) {
    if (g_ephemeralStack.empty()) return;
    if (!isObject(v)) return;
    void* p = asObjectPtr(v);
    auto& top = g_ephemeralStack.back();
    for (auto it = top.begin(); it != top.end(); ++it) {
        if (*it == p) { top.erase(it); break; }
    }
    auto ait = g_arrayRegistry.find(p);
    if (ait != g_arrayRegistry.end()) {
        const auto& elems = ait->second->elements();
        for (const auto& e : elems) gcEphemeralEscapeDeep(e);
        return;
    }
    auto mit = g_hashMapRegistry.find(p);
    if (mit != g_hashMapRegistry.end()) {
        for (const auto& kv : mit->second->data) gcEphemeralEscapeDeep(kv.second);
        return;
    }
    auto iit = g_instanceRegistry.find(p);
    if (iit != g_instanceRegistry.end()) {
        iit->second->forEachField([](Value v){ gcEphemeralEscapeDeep(v); });
        return;
    }
}
void gcEphemeralFrameLeave() {
    if (g_ephemeralStack.empty()) return;
    auto list = std::move(g_ephemeralStack.back());
    g_ephemeralStack.pop_back();
    for (void* p : list) {
        auto ait = g_arrayRegistry.find(p);
        if (ait != g_arrayRegistry.end()) {
            auto arr = std::move(ait->second);
            ait = g_arrayRegistry.erase(ait), void();
            gcReleaseArrayToPool(std::move(arr));
            g_objectGeneration.erase(p);
            continue;
        }
        auto mit = g_hashMapRegistry.find(p);
        if (mit != g_hashMapRegistry.end()) {
            auto map = std::move(mit->second);
            mit = g_hashMapRegistry.erase(mit), void();
            gcReleaseHashMapToPool(std::move(map));
            g_objectGeneration.erase(p);
            continue;
        }
    }
}

void gcSetBenchmarkMode(bool enable) { g_benchmarkMode.store(enable, std::memory_order_relaxed); }
uint64_t gcGetYoungAllocations() { return g_youngAllocations.load(std::memory_order_relaxed); }

} // namespace claw
