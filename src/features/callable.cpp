#include "callable.h"
#include "interpreter.h"
#include "stmt.h"
#include "environment.h"
#include "stack_trace.h"
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
    
    // Push to call stack
    try {
        interpreter.getCallStack().push(declaration_->name, declaration_->token.line);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(declaration_->token, ErrorCode::STACK_OVERFLOW, e.what(), interpreter.getCallStack().get_frames());
    }
    
    // Execute the function body
    try {
        interpreter.executeBlock(declaration_->body, environment);
        interpreter.getCallStack().pop();
    } catch (const ReturnValue& returnValue) {
        // Return statement throws a special exception with the value
        interpreter.getCallStack().pop();
        return returnValue.value;
    } catch (...) {
        // Ensure we pop even on other exceptions (like RuntimeErrors)
        interpreter.getCallStack().pop();
        throw;
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
    try {
        interpreter.getCallStack().push(name_, -1); // Native functions don't have a line number
    } catch (const std::runtime_error&) {
        // For native functions, we don't have a token easily available here
        // but it will likely be caught by the caller's push
        throw;
    }

    try {
        Value result = function_(arguments);
        interpreter.getCallStack().pop();
        return result;
    } catch (...) {
        interpreter.getCallStack().pop();
        throw;
    }
}

int NativeFunction::arity() const {
    return arity_;
}

std::string NativeFunction::toString() const {
    return "<native fn " + name_ + ">";
}

} // namespace volt
