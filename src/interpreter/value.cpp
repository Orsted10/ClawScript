#include "value.h"
#include "callable.h"
#include "array.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace volt {

bool isTruthy(const Value& v) {
    if (isNil(v)) return false;
    if (isBool(v)) return asBool(v);
    if (isNumber(v)) return asNumber(v) != 0.0;
    if (isString(v)) return !asString(v).empty();
    if (isArray(v)) return asArray(v)->length() > 0;
    return true;
}

bool isEqual(const Value& a, const Value& b) {
    if (isNil(a) && isNil(b)) return true;
    if (isNil(a) || isNil(b)) return false;
    
    if (isNumber(a) && isNumber(b)) {
        return asNumber(a) == asNumber(b);
    }
    if (isString(a) && isString(b)) {
        return asString(a) == asString(b);
    }
    if (isBool(a) && isBool(b)) {
        return asBool(a) == asBool(b);
    }
    if (isCallable(a) && isCallable(b)) {
        return std::get<std::shared_ptr<Callable>>(a) ==
               std::get<std::shared_ptr<Callable>>(b);
    }
    // Arrays compare by reference
    if (isArray(a) && isArray(b)) {
        return asArray(a) == asArray(b);
    }
    
    return false;
}

std::string valueToString(const Value& v) {
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
        auto func = std::get<std::shared_ptr<Callable>>(v);
        return func->toString();
    } else if (isArray(v)) {
        return asArray(v)->toString();
    }
    return "unknown";
}

} // namespace volt
