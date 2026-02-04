#pragma once
#include "value.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

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
    
    // Size operations
    size_t size() const { return elements_.size(); }
    int length() const { return static_cast<int>(elements_.size()); }
    
    // Iteration
    const std::vector<Value>& elements() const { return elements_; }
    
    // String representation
    std::string toString() const;
    std::string toStringWithCycleDetection(std::set<const void*>& visited) const;
    
    // Functional methods for chaining
    std::shared_ptr<VoltArray> map(std::function<Value(Value)> func) const;
    std::shared_ptr<VoltArray> filter(std::function<bool(Value)> predicate) const;
    Value reduce(std::function<Value(Value, Value)> reducer, Value initialValue) const;
    
    // Additional array methods
    std::string join(const std::string& separator = ", ") const;
    std::shared_ptr<VoltArray> concat(const std::shared_ptr<VoltArray>& other) const;
    std::shared_ptr<VoltArray> slice(int start, int end = -1) const;
    std::shared_ptr<VoltArray> flat() const;
    std::shared_ptr<VoltArray> flatMap(std::function<Value(Value)> func) const;
    Value find(std::function<bool(Value)> predicate) const;
    bool some(std::function<bool(Value)> predicate) const;
    bool every(std::function<bool(Value)> predicate) const;
    void forEach(std::function<void(Value)> func) const;
    
    // Advanced array methods
    int indexOf(const Value& value) const;
    int lastIndexOf(const Value& value) const;
    std::shared_ptr<VoltArray> sort(std::function<bool(const Value&, const Value&)> comparator = nullptr) const;
    std::shared_ptr<VoltArray> splice(int start, int deleteCount = 0, const std::vector<Value>& items = {}) const;
    Value shift();
    void unshift(const Value& value);
    
private:
    std::vector<Value> elements_;
};

} // namespace volt
