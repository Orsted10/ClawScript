#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <cstring>

namespace volt {

// Forward declarations
class Callable;
class VoltArray;
struct VoltHashMap;
class VoltClass;
class VoltInstance;

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
Value arrayValue(std::shared_ptr<VoltArray> arr);
Value hashMapValue(std::shared_ptr<VoltHashMap> map);
Value classValue(std::shared_ptr<VoltClass> cls);
Value instanceValue(std::shared_ptr<VoltInstance> inst);

// Legacy-compatible helpers (will be updated as we refactor)
bool isTruthy(Value v);
bool isEqual(Value v1, Value v2);
std::string valueToString(const Value& v);
std::string valueToStringWithCycleDetection(const Value& v, std::set<const void*>& visited);

// Specialized object checks (to be refined)
bool isCallable(Value v);
bool isArray(Value v);
bool isHashMap(Value v);
bool isClass(Value v);
bool isInstance(Value v);

std::shared_ptr<VoltArray> asArray(Value v);
std::shared_ptr<VoltHashMap> asHashMap(Value v);
std::shared_ptr<VoltClass> asClass(Value v);
std::shared_ptr<VoltInstance> asInstance(Value v);
std::shared_ptr<Callable> asCallable(Value v);

} // namespace volt
