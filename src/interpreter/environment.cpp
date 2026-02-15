#include "environment.h"
#include "errors.h"
#include "features/string_pool.h"
#include <stdexcept>

namespace volt {

void Environment::define(std::string_view name, Value value) {
    // Intern the name to ensure it has a stable lifetime and fast comparison
    values_[StringPool::intern(name)] = value;
    lookup_cache_.clear(); // Invalidate cache on new definitions
}

void Environment::define(std::string_view name, std::shared_ptr<Callable> fn) {
    values_[StringPool::intern(name)] = callableValue(std::move(fn));
    lookup_cache_.clear();
}
Value Environment::get(std::string_view name) const {  
    // Intern the name to ensure pointer-based comparison works
    name = StringPool::intern(name);

    // 0. Check cache
    auto cache_it = lookup_cache_.find(name);
    if (cache_it != lookup_cache_.end()) {
        return cache_it->second.value;
    }

    // 1. Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        Value val = it->second;
        lookup_cache_[name] = {nullptr, val, true};
        return val;
    }
    
    // 2. Check enclosing scope
    if (enclosing_) {
        try {
            Value val = enclosing_->get(name);
            lookup_cache_[name] = {enclosing_, val, true};
            return val;
        } catch (const VoltError&) {
            // Re-throw if not found in parent
            throw;
        }
    }
    
    throw VoltError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + std::string(name));
}

void Environment::assign(std::string_view name, Value value) {
    // Intern the name to ensure pointer-based comparison works
    name = StringPool::intern(name);

    // Check current scope
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        lookup_cache_[name] = {nullptr, value, true}; // Update cache
        return;
    }
    
    // Check enclosing scope
    if (enclosing_) {
        enclosing_->assign(name, value);
        lookup_cache_[name] = {enclosing_, value, true}; // Update cache
        return;
    }
    
    throw VoltError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + std::string(name));
}

bool Environment::exists(std::string_view name) const {
    name = StringPool::intern(name);
    if (values_.find(name) != values_.end()) {
        return true;
    }
    
    if (enclosing_) {
        return enclosing_->exists(name);
    }
    
    return false;
}

} // namespace volt
