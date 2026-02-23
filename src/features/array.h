#pragma once
#include "value.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace claw {

class ClawArray {
public:
    ClawArray() = default;
    explicit ClawArray(std::vector<Value> elements);
    
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
    std::shared_ptr<ClawArray> map(std::function<Value(Value)> func) const;
    std::shared_ptr<ClawArray> filter(std::function<bool(Value)> predicate) const;
    Value reduce(std::function<Value(Value, Value)> reducer, Value initialValue) const;
    
    // Additional array methods
    std::string join(const std::string& separator = ", ") const;
    std::shared_ptr<ClawArray> concat(const std::shared_ptr<ClawArray>& other) const;
    std::shared_ptr<ClawArray> slice(int start, int end = -1) const;
    std::shared_ptr<ClawArray> flat() const;
    std::shared_ptr<ClawArray> flatMap(std::function<Value(Value)> func) const;
    Value find(std::function<bool(Value)> predicate) const;
    bool some(std::function<bool(Value)> predicate) const;
    bool every(std::function<bool(Value)> predicate) const;
    void forEach(std::function<void(Value)> func) const;
    
    // Advanced array methods
    int indexOf(const Value& value) const;
    int lastIndexOf(const Value& value) const;
    std::shared_ptr<ClawArray> sort(std::function<bool(const Value&, const Value&)> comparator = nullptr) const;
    std::shared_ptr<ClawArray> splice(int start, int deleteCount = 0, const std::vector<Value>& items = {}) const;
    Value shift();
    void unshift(const Value& value);
    void reserve(size_t n) { elements_.reserve(n); }
    void fill(Value v, size_t n);
    void clear() { elements_.clear(); }
    
private:
    std::vector<Value> elements_;
};

} // namespace claw
