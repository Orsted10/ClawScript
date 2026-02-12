#pragma once
#include "value.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace volt {

// Variable storage and scoping
class Environment {
public:
    Environment() : enclosing_(nullptr) {}
    explicit Environment(std::shared_ptr<Environment> enclosing) 
        : enclosing_(enclosing) {}
    
    // Define new variable
    void define(const std::string& name, Value value);
    
    // Get variable value
    Value get(const std::string& name) const;
    
    // Assign to existing variable
    void assign(const std::string& name, Value value);
    
    // Check if variable exists
    bool exists(const std::string& name) const;

    // Environment Caching (v0.8.6 Optimization)
    struct CacheEntry {
        std::shared_ptr<Environment> env;
        Value value;
        bool found;
    };
    
    static void clearGlobalCache();

private:
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment> enclosing_;
    
    // Performance: Fast lookup cache for deeply nested environments
    mutable std::unordered_map<std::string, CacheEntry> lookup_cache_;
};

} // namespace volt
