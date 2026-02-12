#include "environment.h"
#include "errors.h"
#include <stdexcept>

namespace volt {

void Environment::define(const std::string& name, Value value) {
    values_[name] = value;
    lookup_cache_.clear(); // Invalidate cache on new definitions
}

Value Environment::get(const std::string& name) const {  
    // 1. Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }
    
    // 2. Check enclosing scope
    if (enclosing_) {
        return enclosing_->get(name);
    }
    
    throw VoltError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + name);
}

void Environment::assign(const std::string& name, Value value) {
    // Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        lookup_cache_.erase(name); // Invalidate local cache entry
        return;
    }
    
    // Check enclosing scope
    if (enclosing_) {
        enclosing_->assign(name, value);
        lookup_cache_.erase(name); // Invalidate local cache entry
        return;
    }
    
    throw VoltError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + name);
}

bool Environment::exists(const std::string& name) const {
    if (values_.find(name) != values_.end()) {
        return true;
    }
    
    if (enclosing_) {
        return enclosing_->exists(name);
    }
    
    return false;
}

} // namespace volt
