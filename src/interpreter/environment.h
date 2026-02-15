#pragma once
#include "value.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <string_view>

namespace volt {

// Variable storage and scoping
class Environment {
public:
    Environment() : enclosing_(nullptr) {}
    explicit Environment(std::shared_ptr<Environment> enclosing) 
        : enclosing_(enclosing) {}
    
    // Define new variable
    void define(std::string_view name, Value value);
    void define(std::string_view name, std::shared_ptr<class Callable> fn);
    
    // Get variable value
    Value get(std::string_view name) const;
    
    // Assign to existing variable
    void assign(std::string_view name, Value value);
    
    // Check if variable exists
    bool exists(std::string_view name) const;

    // Environment Caching (v0.8.6 Optimization)
    struct CacheEntry {
        std::shared_ptr<Environment> env;
        Value value;
        bool found;
    };

    struct ProfileEntry {
        uint64_t hits;
    };

    struct InternedStringHash {
        size_t operator()(std::string_view sv) const {
            return std::hash<const char*>{}(sv.data());
        }
    };

    struct InternedStringEqual {
        bool operator()(std::string_view sv1, std::string_view sv2) const {
            return sv1.data() == sv2.data();
        }
    };
    
    static void clearGlobalCache();

private:
    // Using string_view as key for performance (guaranteed interned)
    std::unordered_map<std::string_view, Value, InternedStringHash, InternedStringEqual> values_;
    std::shared_ptr<Environment> enclosing_;
    
    // Performance: Fast lookup cache for deeply nested environments
    mutable std::unordered_map<std::string_view, CacheEntry, InternedStringHash, InternedStringEqual> lookup_cache_;
    mutable std::unordered_map<std::string_view, ProfileEntry, InternedStringHash, InternedStringEqual> profile_;
};

} // namespace volt
