#include "value.h"
#include <sstream>
#include <iomanip>

namespace volt {

bool isTruthy(const Value& v) {
    if (isNil(v)) return false;
    if (isBool(v)) return asBool(v);
    return true; // Everything else is truthy
}

bool isEqual(const Value& a, const Value& b) {
    // Nil equality
    if (isNil(a) && isNil(b)) return true;
    if (isNil(a)) return false;
    
    // Type mismatch
    if (a.index() != b.index()) return false;
    
    // Compare by type
    if (isBool(a)) return asBool(a) == asBool(b);
    if (isNumber(a)) return asNumber(a) == asNumber(b);
    if (isString(a)) return asString(a) == asString(b);
    
    return false;
}

std::string valueToString(const Value& v) {
    if (isNil(v)) {
        return "nil";
    }
    
    if (isBool(v)) {
        return asBool(v) ? "true" : "false";
    }
    
    if (isNumber(v)) {
        std::ostringstream oss;
        double num = asNumber(v);
        
        // Check if integer
        if (num == static_cast<long long>(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << std::fixed << std::setprecision(6) << num;
        }
        return oss.str();
    }
    
    if (isString(v)) {
        return asString(v);
    }
    
    if (isCallable(v)) {
        return "<function>";
    }
    
    return "<unknown>";
}

} // namespace volt
