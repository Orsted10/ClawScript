#include "array.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

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
    if (index >= elements_.size()) {
        throw std::runtime_error("Array index out of bounds: " + 
                                std::to_string(index));
    }
    elements_[index] = value;
}

void VoltArray::push(Value value) {
    elements_.push_back(value);
}

Value VoltArray::pop() {
    if (elements_.empty()) {
        return nullptr;  // Return nil for empty array
    }
    Value last = elements_.back();
    elements_.pop_back();
    return last;
}

void VoltArray::reverse() {
    std::reverse(elements_.begin(), elements_.end());
}

std::string VoltArray::toString() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements_.size(); i++) {
        if (i > 0) oss << ", ";
        oss << valueToString(elements_[i]);
    }
    oss << "]";
    return oss.str();
}

} // namespace volt
