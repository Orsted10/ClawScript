#include "class.h"
#include "interpreter.h"
#include "interpreter/gc_alloc.h"
#include "errors.h"

namespace claw {

// ========================================
// ClawClass Implementation
// ========================================

std::shared_ptr<ClawFunction> ClawClass::findMethod(const std::string& name) const {
    auto it = methods_.find(name);
    if (it != methods_.end()) {
        return it->second;
    }

    if (superclass_) {
        return superclass_->findMethod(name);
    }

    return nullptr;
}

Value ClawClass::call(Interpreter& interpreter, const std::vector<Value>& arguments) {
    auto instance = gcNewInstance(shared_from_this());
    
    // Look for initializer
    auto initializer = findMethod("init");
    if (initializer) {
        // Bind 'this' to the instance and call init
        initializer->bind(instance)->call(interpreter, arguments);
    }

    return instanceValue(instance);
}

int ClawClass::arity() const {
    auto initializer = findMethod("init");
    if (initializer) {
        return initializer->arity();
    }
    return 0;
}

// ========================================
// ClawInstance Implementation
// ========================================

Value ClawInstance::get(const Token& name) {
    std::string_view sv = StringPool::intern(name.lexeme);
    
    // 1. Check fields
    auto it = fields_.find(sv);
    if (it != fields_.end()) {
        return it->second;
    }

    // 2. Check methods
    auto cacheIt = ic_get_cache_.find(sv);
    if (cacheIt != ic_get_cache_.end()) {
        return cacheIt->second;
    }
    auto method = class_->findMethod(std::string(sv));
    if (method) {
        Value bound = callableValue(method->bind(shared_from_this()));
        ic_get_cache_[sv] = bound;
        return bound;
    }

    throw RuntimeError(name, ErrorCode::RUNTIME_ERROR, "Undefined property '" + std::string(sv) + "'.", {});
}

void ClawInstance::set(const Token& name, Value value) {
    std::string_view sv = StringPool::intern(name.lexeme);
    gcBarrierWrite(this, value);
    fields_[sv] = value;
}

void ClawInstance::forEachField(const std::function<void(Value)>& fn) const {
    for (const auto& kv : fields_) fn(kv.second);
}
bool ClawInstance::has(const Token& name) const {
    std::string_view sv = StringPool::intern(name.lexeme);
    return fields_.find(sv) != fields_.end();
}

} // namespace claw
