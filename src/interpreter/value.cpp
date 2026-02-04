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

bool isTruthy(const Value& v) {
    if (isNil(v)) return false;
    if (isBool(v)) return asBool(v);
    if (isNumber(v)) return asNumber(v) != 0.0;
    if (isString(v)) return !asString(v).empty();
    if (isArray(v)) return asArray(v)->length() > 0;
    if (isHashMap(v)) return asHashMap(v)->size() > 0;  // Added!
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
        auto func = std::get<std::shared_ptr<Callable>>(v);
        return func->toString();
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

} // namespace volt