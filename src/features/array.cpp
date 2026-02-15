#include "array.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <set>

namespace volt {

VoltArray::VoltArray(std::vector<Value> elements)
    : elements_(std::move(elements)) {}

Value VoltArray::get(size_t index) const {
    if (index >= elements_.size()) {
        throw std::runtime_error("Array index out of bounds: " + 
                                std::to_string(index));
    }
    return elements_[index];
}

void VoltArray::set(size_t index, Value value) {
    // Prevent integer overflow and unreasonable array sizes
    if (index >= 1000000) {  // Reasonable upper limit
        throw std::runtime_error("Array index too large: " + std::to_string(index));
    }
    if (index >= elements_.size()) {
        // Extend array with nil values if needed
        if (index >= elements_.size() + 10000) {  // Prevent massive allocations
            throw std::runtime_error("Array extension too large: " + std::to_string(index));
        }
        elements_.resize(index + 1, volt::nilValue());
    }
    elements_[index] = value;
}

void VoltArray::push(Value value) {
    elements_.push_back(value);
}

Value VoltArray::pop() {
    if (elements_.empty()) {
        return volt::nilValue();  // Return nil for empty array
    }
    Value last = elements_.back();
    elements_.pop_back();
    return last;
}

void VoltArray::reverse() {
    std::reverse(elements_.begin(), elements_.end());
}

std::string VoltArray::toString() const {
    std::set<const void*> dummy_visited;
    return toStringWithCycleDetection(dummy_visited);
}

std::string VoltArray::toStringWithCycleDetection(std::set<const void*>& visited) const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements_.size(); i++) {
        if (i > 0) oss << ", ";
        oss << valueToStringWithCycleDetection(elements_[i], visited);
    }
    oss << "]";
    return oss.str();
}

std::shared_ptr<VoltArray> VoltArray::map(std::function<Value(Value)> func) const {
    auto result = std::make_shared<VoltArray>();
    result->elements_.reserve(elements_.size());  // Pre-allocate memory
    for (const auto& element : elements_) {
        result->elements_.push_back(func(element));
    }
    return result;
}

std::shared_ptr<VoltArray> VoltArray::filter(std::function<bool(Value)> predicate) const {
    auto result = std::make_shared<VoltArray>();
    result->elements_.reserve(elements_.size() / 2);  // Conservative pre-allocation
    for (const auto& element : elements_) {
        if (predicate(element)) {
            result->elements_.push_back(element);
        }
    }
    return result;
}

Value VoltArray::reduce(std::function<Value(Value, Value)> reducer, Value initialValue) const {
    Value accumulator = initialValue;
    for (const auto& element : elements_) {
        accumulator = reducer(accumulator, element);
    }
    return accumulator;
}

std::shared_ptr<VoltArray> VoltArray::slice(int start, int end) const {
    auto result = std::make_shared<VoltArray>();
    
    // Handle negative indices
    if (start < 0) start = elements_.size() + start;
    if (end < 0) end = elements_.size() + end;
    
    // Clamp to valid range
    start = std::max(0, std::min(static_cast<int>(elements_.size()), start));
    if (end == -1) end = elements_.size();
    end = std::max(start, std::min(static_cast<int>(elements_.size()), end));
    
    // Copy elements
    for (int i = start; i < end; i++) {
        result->push(elements_[i]);
    }
    
    return result;
}

std::shared_ptr<VoltArray> VoltArray::concat(const std::shared_ptr<VoltArray>& other) const {
    auto result = std::make_shared<VoltArray>(elements_);
    for (const auto& element : other->elements()) {
        result->push(element);
    }
    return result;
}

std::string VoltArray::join(const std::string& separator) const {
    std::ostringstream oss;
    for (size_t i = 0; i < elements_.size(); i++) {
        if (i > 0) oss << separator;
        oss << valueToString(elements_[i]);
    }
    return oss.str();
}

Value VoltArray::find(std::function<bool(Value)> predicate) const {
    for (const auto& element : elements_) {
        if (predicate(element)) {
            return element;
        }
    }
    return volt::nilValue(); // Return nil if not found
}

bool VoltArray::some(std::function<bool(Value)> predicate) const {
    for (const auto& element : elements_) {
        if (predicate(element)) {
            return true;
        }
    }
    return false;
}

bool VoltArray::every(std::function<bool(Value)> predicate) const {
    for (const auto& element : elements_) {
        if (!predicate(element)) {
            return false;
        }
    }
    return true;
}

void VoltArray::forEach(std::function<void(Value)> func) const {
    for (const auto& element : elements_) {
        func(element);
    }
}

int VoltArray::indexOf(const Value& value) const {
    for (size_t i = 0; i < elements_.size(); i++) {
        if (isEqual(elements_[i], value)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int VoltArray::lastIndexOf(const Value& value) const {
    for (int i = static_cast<int>(elements_.size()) - 1; i >= 0; i--) {
        if (isEqual(elements_[i], value)) {
            return i;
        }
    }
    return -1;
}

std::shared_ptr<VoltArray> VoltArray::sort(std::function<bool(const Value&, const Value&)> comparator) const {
    auto result = std::make_shared<VoltArray>(elements_);
    
    if (comparator) {
        // Custom comparator
        std::sort(result->elements_.begin(), result->elements_.end(), 
                  [&comparator](const Value& a, const Value& b) {
                      return comparator(a, b);
                  });
    } else {
        // Default sorting (numbers first, then strings, then others)
        std::sort(result->elements_.begin(), result->elements_.end(),
                  [](const Value& a, const Value& b) -> bool {
                      if (isNumber(a) && isNumber(b)) {
                          return asNumber(a) < asNumber(b);
                      }
                      if (isString(a) && isString(b)) {
                          return asString(a) < asString(b);
                      }
                      if (isNumber(a)) return true;
                      if (isString(a)) return isNumber(b) ? false : true;
                      return false;
                  });
    }
    
    return result;
}

std::shared_ptr<VoltArray> VoltArray::splice(int start, int deleteCount, const std::vector<Value>& items) const {
    auto result = std::make_shared<VoltArray>(elements_);
    
    // Handle negative indices
    if (start < 0) start = elements_.size() + start;
    start = std::max(0, std::min(static_cast<int>(elements_.size()), start));
    
    // Clamp deleteCount
    deleteCount = std::max(0, std::min(deleteCount, static_cast<int>(elements_.size()) - start));
    
    // Remove elements
    result->elements_.erase(result->elements_.begin() + start, 
                           result->elements_.begin() + start + deleteCount);
    
    // Insert new elements
    result->elements_.insert(result->elements_.begin() + start, items.begin(), items.end());
    
    return result;
}

Value VoltArray::shift() {
    if (elements_.empty()) return volt::nilValue();
    Value first = elements_[0];
    elements_.erase(elements_.begin());
    return first;
}

void VoltArray::unshift(const Value& value) {
    elements_.insert(elements_.begin(), value);
}

std::shared_ptr<VoltArray> VoltArray::flat() const {
    auto result = std::make_shared<VoltArray>();
    
    for (const auto& element : elements_) {
        if (isArray(element)) {
            auto nestedArray = asArray(element);
            for (size_t i = 0; i < nestedArray->size(); i++) {
                result->push(nestedArray->get(i));
            }
        } else {
            result->push(element);
        }
    }
    
    return result;
}

std::shared_ptr<VoltArray> VoltArray::flatMap(std::function<Value(Value)> func) const {
    auto result = std::make_shared<VoltArray>();
    
    for (const auto& element : elements_) {
        Value mapped = func(element);
        if (isArray(mapped)) {
            auto nestedArray = asArray(mapped);
            for (size_t i = 0; i < nestedArray->size(); i++) {
                result->push(nestedArray->get(i));
            }
        } else {
            result->push(mapped);
        }
    }
    
    return result;
}

} // namespace volt
