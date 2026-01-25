#pragma once
#include "value.h"
#include <vector>
#include <memory>
#include <string>

namespace volt {

/**
 * VoltArray - Native array implementation
 * 
 * Arrays in VoltScript are:
 * - Dynamic (can grow/shrink),
 * - Heterogeneous (can hold mixed types)
 * - Zero-indexed
 * - Have built-in methods (push, pop, length, etc.)
 */
class VoltArray {
public:
    VoltArray() = default;
    explicit VoltArray(std::vector<Value> elements);
    
    // Element access
    Value get(size_t index) const;
    void set(size_t index, Value value);
    
    // Array operations
    void push(Value value);
    Value pop();
    void reverse();
    size_t length() const { return elements_.size(); }
    
    // Iteration
    const std::vector<Value>& elements() const { return elements_; }
    
    // String representation
    std::string toString() const;
    
private:
    std::vector<Value> elements_;
};

} // namespace volt
