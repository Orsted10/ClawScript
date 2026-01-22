#pragma once
#include "value.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>  // ← ADD THIS!

namespace volt {

// Forward declarations
class Interpreter;
class Environment;

/**
 * Callable - Base interface for anything that can be called like a function
 * 
 * In VoltScript, functions are first-class citizens - they can be:
 * - Passed as arguments
 * - Returned from other functions
 * - Stored in variables
 * 
 * This interface allows both user-defined functions and native functions
 * to work the same way in the interpreter.
 */
class Callable {
public:
    virtual ~Callable() = default;
    
    // Execute the function with given arguments
    virtual Value call(Interpreter& interpreter, 
                      const std::vector<Value>& arguments) = 0;
    
    // How many parameters does this function expect?
    virtual int arity() const = 0;
    
    // String representation (for debugging)
    virtual std::string toString() const = 0;
};

/**
 * VoltFunction - User-defined functions from VoltScript code
 * 
 * These are functions written in VoltScript itself (using 'fn' keyword).
 * They capture their surrounding environment to support closures.
 */
class VoltFunction : public Callable {
public:
    VoltFunction(class FnStmt* declaration, 
                 std::shared_ptr<Environment> closure);
    
    Value call(Interpreter& interpreter, 
              const std::vector<Value>& arguments) override;
    
    int arity() const override;
    std::string toString() const override;
    
private:
    class FnStmt* declaration_;           // The function's AST node
    std::shared_ptr<Environment> closure_; // The environment where it was defined
};

/**
 * NativeFunction - Built-in functions implemented in C++
 * 
 * These are functions provided by the language runtime (like 'clock()').
 * They're implemented in C++ for performance or to access system features.
 */
class NativeFunction : public Callable {
public:
    using NativeFn = std::function<Value(const std::vector<Value>&)>;
    
    NativeFunction(int arity, NativeFn function, std::string name);
    
    Value call(Interpreter& interpreter, 
              const std::vector<Value>& arguments) override;
    
    int arity() const override;
    std::string toString() const override;
    
private:
    int arity_;
    NativeFn function_;
    std::string name_;
};

} // namespace volt
