#include "callable.h"
#include "interpreter.h"
#include "stmt.h"
#include "environment.h"
#include <sstream>

namespace volt {

// ========================================
// VoltFunction (User-defined functions)
// ========================================

VoltFunction::VoltFunction(FnStmt* declaration, 
                           std::shared_ptr<Environment> closure)
    : declaration_(declaration), closure_(closure) {}

Value VoltFunction::call(Interpreter& interpreter, 
                        const std::vector<Value>& arguments) {
    // Create a new environment for this function call
    // The closure is the parent (so we can access captured variables)
    auto environment = std::make_shared<Environment>(closure_);
    
    // Bind parameters to arguments
    for (size_t i = 0; i < declaration_->parameters.size(); i++) {
        environment->define(declaration_->parameters[i], arguments[i]);
    }
    
    // Execute the function body
    try {
        interpreter.executeBlock(declaration_->body, environment);
    } catch (const ReturnValue& returnValue) {
        // Return statement throws a special exception with the value
        return returnValue.value;
    }
    
    // If no return statement, functions return nil
    return nullptr;
}

int VoltFunction::arity() const {
    return static_cast<int>(declaration_->parameters.size());
}

std::string VoltFunction::toString() const {
    return "<fn " + declaration_->name + ">";
}

// ========================================
// NativeFunction (Built-in C++ functions)
// ========================================

NativeFunction::NativeFunction(int arity, NativeFn function, std::string name)
    : arity_(arity), function_(function), name_(std::move(name)) {}

Value NativeFunction::call(Interpreter& interpreter, 
                          const std::vector<Value>& arguments) {
    // Just call the C++ function we wrapped
    return function_(arguments);
}

int NativeFunction::arity() const {
    return arity_;
}

std::string NativeFunction::toString() const {
    return "<native fn " + name_ + ">";
}

} // namespace volt
