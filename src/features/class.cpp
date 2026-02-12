#include "class.h"
#include "interpreter.h"
#include "errors.h"

namespace volt {

// ========================================
// VoltClass Implementation
// ========================================

std::shared_ptr<VoltFunction> VoltClass::findMethod(const std::string& name) const {
    auto it = methods_.find(name);
    if (it != methods_.end()) {
        return it->second;
    }

    if (superclass_) {
        return superclass_->findMethod(name);
    }

    return nullptr;
}

Value VoltClass::call(Interpreter& interpreter, const std::vector<Value>& arguments) {
    auto instance = std::make_shared<VoltInstance>(shared_from_this());
    
    // Look for initializer
    auto initializer = findMethod("init");
    if (initializer) {
        // Bind 'this' to the instance and call init
        initializer->bind(instance)->call(interpreter, arguments);
    }

    return instance;
}

int VoltClass::arity() const {
    auto initializer = findMethod("init");
    if (initializer) {
        return initializer->arity();
    }
    return 0;
}

// ========================================
// VoltInstance Implementation
// ========================================

Value VoltInstance::get(const Token& name) {
    std::string fieldName(name.lexeme);
    
    // 1. Check fields
    auto it = fields_.find(fieldName);
    if (it != fields_.end()) {
        return it->second;
    }

    // 2. Check methods
    auto method = class_->findMethod(fieldName);
    if (method) {
        // Bind 'this' to this instance
        return method->bind(shared_from_this());
    }

    throw RuntimeError(name, ErrorCode::RUNTIME_ERROR, "Undefined property '" + fieldName + "'.", {});
}

void VoltInstance::set(const Token& name, Value value) {
    fields_[std::string(name.lexeme)] = value;
}

} // namespace volt
