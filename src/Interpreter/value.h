#pragma once
#include <string>
#include <memory>
#include <variant>
#include <vector>

namespace volt {

// Forward declarations
class Callable;

//Ooooh-- so this is the implementation of the let ... using std::variant hmmm
// Runtime value types
using Value = std::variant<
    std::nullptr_t,              // nil
    bool,                        // boolean
    double,                      // number
    std::string,                 // string
    std::shared_ptr<Callable>    // function (for future use)
>;

// Type checking helpers
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

// Get typed values
inline double asNumber(const Value& v) {
    return std::get<double>(v);
}

inline bool asBool(const Value& v) {
    return std::get<bool>(v);
}

inline std::string asString(const Value& v) {
    return std::get<std::string>(v);
}

// Truthiness (for conditionals)
bool isTruthy(const Value& v);

// Equality
bool isEqual(const Value& a, const Value& b);

// String representation
std::string valueToString(const Value& v);

} // namespace volt
