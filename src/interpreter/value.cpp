#include "value.h"
#include "callable.h"
#include "array.h"
#include "features/hashmap.h"  // Added!
#include <sstream>
#include <iomanip>
#include <cmath>
#include <set>

namespace volt {

// Thread-local set to track visited objects for cycle detection during string conversion
thread_local std::set<const void*> visitedObjects;

// Global registries to keep shared_ptr lifetimes while storing raw pointers in Values
static std::unordered_map<void*, std::shared_ptr<Callable>> g_callableRegistry;
static std::unordered_map<void*, std::shared_ptr<VoltArray>> g_arrayRegistry;
static std::unordered_map<void*, std::shared_ptr<VoltHashMap>> g_hashMapRegistry;
static std::unordered_map<void*, std::shared_ptr<VoltClass>> g_classRegistry;
static std::unordered_map<void*, std::shared_ptr<VoltInstance>> g_instanceRegistry;

Value callableValue(std::shared_ptr<Callable> fn) {
    void* p = fn.get();
    g_callableRegistry[p] = std::move(fn);
    return objectValue(p);
}
Value arrayValue(std::shared_ptr<VoltArray> arr) {
    void* p = arr.get();
    g_arrayRegistry[p] = std::move(arr);
    return objectValue(p);
}
Value hashMapValue(std::shared_ptr<VoltHashMap> map) {
    void* p = map.get();
    g_hashMapRegistry[p] = std::move(map);
    return objectValue(p);
}
Value classValue(std::shared_ptr<VoltClass> cls) {
    void* p = cls.get();
    g_classRegistry[p] = std::move(cls);
    return objectValue(p);
}
Value instanceValue(std::shared_ptr<VoltInstance> inst) {
    void* p = inst.get();
    g_instanceRegistry[p] = std::move(inst);
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
            oss << static_cast<long long>(num);
        } else {
            oss << std::fixed << std::setprecision(6) << num;
            std::string str = oss.str();
            // Remove trailing zeros
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') str.pop_back();
            return str;
        }
        return oss.str();
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

bool isCallable(Value v) { return isObject(v) && g_callableRegistry.count(asObjectPtr(v)) > 0; }
bool isArray(Value v) { return isObject(v) && g_arrayRegistry.count(asObjectPtr(v)) > 0; }
bool isHashMap(Value v) { return isObject(v) && g_hashMapRegistry.count(asObjectPtr(v)) > 0; }
bool isClass(Value v) { return isObject(v) && g_classRegistry.count(asObjectPtr(v)) > 0; }
bool isInstance(Value v) { return isObject(v) && g_instanceRegistry.count(asObjectPtr(v)) > 0; }

std::shared_ptr<VoltArray> asArray(Value v) { auto it = g_arrayRegistry.find(asObjectPtr(v)); return it != g_arrayRegistry.end() ? it->second : nullptr; }
std::shared_ptr<VoltHashMap> asHashMap(Value v) { auto it = g_hashMapRegistry.find(asObjectPtr(v)); return it != g_hashMapRegistry.end() ? it->second : nullptr; }
std::shared_ptr<VoltClass> asClass(Value v) { auto it = g_classRegistry.find(asObjectPtr(v)); return it != g_classRegistry.end() ? it->second : nullptr; }
std::shared_ptr<VoltInstance> asInstance(Value v) { auto it = g_instanceRegistry.find(asObjectPtr(v)); return it != g_instanceRegistry.end() ? it->second : nullptr; }
std::shared_ptr<Callable> asCallable(Value v) { auto it = g_callableRegistry.find(asObjectPtr(v)); return it != g_callableRegistry.end() ? it->second : nullptr; }

} // namespace volt
