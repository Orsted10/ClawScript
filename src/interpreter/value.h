#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <cstring>

namespace claw {

class Callable;
class ClawArray;
struct ClawHashMap;
class ClawClass;
class ClawInstance;
class Chunk;
struct VMFunction;
struct VMClosure;
struct VMUpvalue;

/**
 * Value - NaN-boxed value implementation for VoltScript
 * 
 * A 64-bit double has 52 bits of fraction. If all 11 exponent bits are 1,
 * it's a NaN. This leaves 52 bits for payload.
 * 
 * We use the quiet NaN bit (bit 51) and 3 bits (50, 49, 48) for tagging.
 * This gives us 48 bits for pointers, which is enough for modern 64-bit systems.
 * 
 * Tagging Scheme:
 * 000: Nil
 * 001: False
 * 010: True
 * 011: String (interned string_view pointer)
 * 100: Object (shared_ptr or raw pointer to VoltObject)
 */

typedef uint64_t Value;

// Constants for NaN boxing
constexpr uint64_t QNAN = 0x7ffc000000000000;
constexpr uint64_t SIGN_BIT = 0x8000000000000000;

constexpr uint64_t TAG_NIL    = 1; // 001
constexpr uint64_t TAG_FALSE  = 2; // 010
constexpr uint64_t TAG_TRUE   = 3; // 011
constexpr uint64_t TAG_STRING = 4; // 100
constexpr uint64_t TAG_OBJECT = 5; // 101

inline uint64_t payload(Value v) { return v & ~(QNAN | 0x7); }
inline uint64_t tagBits(Value v) { return v & (QNAN | 0x7); }
inline bool pointerValid(const void* p) { return (reinterpret_cast<uint64_t>(p) & QNAN) == 0; }

// Value creators
inline Value numberToValue(double num) {
    Value v;
    std::memcpy(&v, &num, sizeof(double));
    return v;
}

inline Value nilValue() {
    return QNAN | TAG_NIL;
}

inline Value boolValue(bool b) {
    return QNAN | (b ? TAG_TRUE : TAG_FALSE);
}

// Objects are stored as pointers. For now, we use raw pointers to shared_ptr or similar.
// To keep it simple and safe, we'll store a pointer to a wrapper or the object itself.
inline Value stringValue(const char* interned_ptr) {
    return QNAN | TAG_STRING | reinterpret_cast<uint64_t>(interned_ptr);
}

inline Value objectValue(void* obj_ptr) {
    return QNAN | TAG_OBJECT | reinterpret_cast<uint64_t>(obj_ptr);
}

struct VMFunction {
    std::string name;
    int arity = 0;
    int upvalueCount = 0;
    std::shared_ptr<Chunk> chunk;
};

struct VMUpvalue {
    Value* location = nullptr;
    Value closed = 0;
};

struct VMClosure {
    std::shared_ptr<VMFunction> function;
    std::vector<std::shared_ptr<VMUpvalue>> upvalues;
};

// Type checks
inline bool isNumber(Value v) { return (v & QNAN) != QNAN; }
inline bool isNil(Value v) { return v == nilValue(); }
inline bool isBool(Value v) { return (v & (QNAN | TAG_FALSE)) == (QNAN | TAG_FALSE) || (v & (QNAN | TAG_TRUE)) == (QNAN | TAG_TRUE); }
inline bool isString(Value v) { return tagBits(v) == (QNAN | TAG_STRING); }
inline bool isObject(Value v) { return tagBits(v) == (QNAN | TAG_OBJECT); }

// Value extractors
inline double asNumber(Value v) {
    double num;
    std::memcpy(&num, &v, sizeof(double));
    return num;
}

inline bool asBool(Value v) {
    return v == (QNAN | TAG_TRUE);
}

inline const char* asStringPtr(Value v) {
    return reinterpret_cast<const char*>(payload(v));
}

inline std::string asString(Value v) {
    const char* p = asStringPtr(v);
    return p ? std::string(p) : std::string();
}

inline void* asObjectPtr(Value v) {
    return reinterpret_cast<void*>(payload(v));
}

// Object Value creators using global registries
Value callableValue(std::shared_ptr<Callable> fn);
Value arrayValue(std::shared_ptr<ClawArray> arr);
Value hashMapValue(std::shared_ptr<ClawHashMap> map);
Value classValue(std::shared_ptr<ClawClass> cls);
Value instanceValue(std::shared_ptr<ClawInstance> inst);
Value vmFunctionValue(std::shared_ptr<VMFunction> fn);
Value vmClosureValue(std::shared_ptr<VMClosure> closure);

// Legacy-compatible helpers (will be updated as we refactor)
bool isTruthy(Value v);
bool isEqual(Value v1, Value v2);
std::string valueToString(const Value& v);
std::string valueToStringWithCycleDetection(const Value& v, std::set<const void*>& visited);
bool diagnosticsEnabled();

// Specialized object checks (to be refined)
bool isCallable(Value v);
bool isArray(Value v);
bool isHashMap(Value v);
bool isClass(Value v);
bool isInstance(Value v);
bool isVMFunction(Value v);
bool isVMClosure(Value v);

std::shared_ptr<ClawArray> asArray(Value v);
std::shared_ptr<ClawHashMap> asHashMap(Value v);
std::shared_ptr<ClawClass> asClass(Value v);
std::shared_ptr<ClawInstance> asInstance(Value v);
std::shared_ptr<Callable> asCallable(Value v);
std::shared_ptr<VMFunction> asVMFunction(Value v);
std::shared_ptr<VMClosure> asVMClosure(Value v);
VMClosure* asVMClosurePtr(Value v);

// GC APIs
void gcRegisterVM(class VM* vm);
void gcUnregisterVM(class VM* vm);
void gcBarrierWrite(const void* parent, Value child);
void gcMaybeCollect();
std::shared_ptr<ClawArray> gcAcquireArrayFromPool();
void gcReleaseArrayToPool(std::shared_ptr<ClawArray> arr);
std::shared_ptr<ClawHashMap> gcAcquireHashMapFromPool();
void gcReleaseHashMapToPool(std::shared_ptr<ClawHashMap> map);
void gcEphemeralFrameEnter();
void gcEphemeralFrameLeave();
void gcEphemeralEscape(Value v);
void gcEphemeralEscapeDeep(Value v);
void gcSetBenchmarkMode(bool enable);
uint64_t gcGetYoungAllocations();

} // namespace claw
