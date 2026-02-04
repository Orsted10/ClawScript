#pragma once
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <set>

namespace volt {

// Forward declarations
class Callable;
class VoltArray;  // Added!
struct VoltHashMap;  // Added - Changed from class to struct to match definition

// Runtime value types
using Value = std::variant<
    std::nullptr_t,              // nil
    bool,                        // boolean
    double,                      // number
    std::string,                 // string
    std::shared_ptr<Callable>,   // function
    std::shared_ptr<VoltArray>,  // array - Added!
    std::shared_ptr<VoltHashMap> // hash map - Added!
>;

// Easy ways to check what type of value we're dealing with
inline bool isNil(const Value& v) {
    return std::holds_alternative<std::nullptr_t>(v);
}

inline bool isBool(const Value& v) {
    return std::holds_alternative<bool>(v);
}

inline bool isNumber(const Value& v) {
    return std::holds_alternative<double>(v);
}

inline bool isString(const Value& v) {
    return std::holds_alternative<std::string>(v);
}

inline bool isCallable(const Value& v) {
    return std::holds_alternative<std::shared_ptr<Callable>>(v);
}

inline bool isArray(const Value& v) {  // Added!
    return std::holds_alternative<std::shared_ptr<VoltArray>>(v);
}

inline bool isHashMap(const Value& v) {  // Added!
    return std::holds_alternative<std::shared_ptr<VoltHashMap>>(v);
}

// Extract actual values from our variant wrapper
inline double asNumber(const Value& v) {
    return std::get<double>(v);
}

inline bool asBool(const Value& v) {
    return std::get<bool>(v);
}

inline std::string asString(const Value& v) {
    return std::get<std::string>(v);
}

inline std::shared_ptr<VoltArray> asArray(const Value& v) {  // Added!
    return std::get<std::shared_ptr<VoltArray>>(v);
}

inline std::shared_ptr<VoltHashMap> asHashMap(const Value& v) {  // Added!
    return std::get<std::shared_ptr<VoltHashMap>>(v);
}

inline std::shared_ptr<Callable> asCallable(const Value& v) {
    return std::get<std::shared_ptr<Callable>>(v);
}

// Figure out if something is true or false for if statements
bool isTruthy(const Value& v);

// Equality
bool isEqual(const Value& a, const Value& b);

// String representation
std::string valueToString(const Value& v);
std::string valueToStringWithCycleDetection(const Value& v, std::set<const void*>& visited);

} // namespace volt