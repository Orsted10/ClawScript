#include "environment.h"
#include <stdexcept>

namespace volt {

void Environment::define(const std::string& name, Value value) {
    values_[name] = value;
}

Value Environment::get(const std::string& name) const {  
    // Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }
    
    // Check enclosing scope
    if (enclosing_) {
        return enclosing_->get(name);
    }
    
    throw std::runtime_error("Undefined variable: " + name);
}

void Environment::assign(const std::string& name, Value value) {
    // Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        return;
    }
    
    // Check enclosing scope
    if (enclosing_) {
        enclosing_->assign(name, value);
        return;
    }
    
    throw std::runtime_error("Undefined variable: " + name);
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
