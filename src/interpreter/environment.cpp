#include "environment.h"
#include "errors.h"
#include "features/string_pool.h"
#include <stdexcept>

namespace claw {

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
        } catch (const ClawError&) {
            // Re-throw if not found in parent
            throw;
        }
    }
    
    throw ClawError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + std::string(name));
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
    
    throw ClawError(ErrorCode::UNDEFINED_VARIABLE, "Undefined variable: " + std::string(name));
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

void Environment::forEachValue(const std::function<void(Value)>& fn) const {
    for (const auto& kv : values_) {
        fn(kv.second);
    }
    if (enclosing_) enclosing_->forEachValue(fn);
}

void Environment::forEachKey(const std::function<void(std::string_view)>& fn) const {
    for (const auto& kv : values_) {
        fn(kv.first);
    }
    if (enclosing_) enclosing_->forEachKey(fn);
}

void Environment::setSandbox(SandboxMode mode) {
    sandboxMode_ = mode;
    switch (mode) {
        case SandboxMode::Full:
            allowFileRead_ = true;
            allowFileWrite_ = true;
            allowFileDelete_ = true;
            allowInput_ = true;
            allowOutput_ = true;
            allowNetwork_ = true;
            break;
        case SandboxMode::Network:
            allowFileRead_ = true;
            allowFileWrite_ = false;
            allowFileDelete_ = false;
            allowInput_ = true;
            allowOutput_ = true;
            allowNetwork_ = true;
            break;
        case SandboxMode::Strict:
            allowFileRead_ = false;
            allowFileWrite_ = false;
            allowFileDelete_ = false;
            allowInput_ = false;
            allowOutput_ = true;
            allowNetwork_ = false;
            break;
    }
}

} // namespace claw
