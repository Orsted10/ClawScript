#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace volt {

// For now we keep Value simple.
// Later this can become variant<number, string, bool, nil>
using Value = double;

class Environment {
private:
    std::unordered_map<std::string, Value> values;
    std::shared_ptr<Environment> enclosing; // parent scope

public:
    // Global scope
    Environment() = default;

    // Nested scope
    explicit Environment(std::shared_ptr<Environment> parent);

    // Define variable in CURRENT scope
    void define(const std::string& name, Value value);

    // Get variable (searches current + parent scopes)
    Value get(const std::string& name) const;

    // Assign variable (updates nearest existing scope)
    void assign(const std::string& name, Value value);

    // Utility
    bool existsInCurrentScope(const std::string& name) const;
};

} // namespace volt
