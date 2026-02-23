#pragma once
#include "value.h"
#include "../features/array.h"
#include "interpreter.h"
#include <functional>

namespace claw {

// Helper class to wrap interpreter calls for array methods
class ArrayMethodHelper {
public:
    ArrayMethodHelper(Interpreter& interp, std::shared_ptr<ClawArray> arr)
        : interpreter_(interp), array_(arr) {}
    
    Value callFunction(const std::shared_ptr<Callable>& func, const std::vector<Value>& args) {
        return func->call(interpreter_, args);
    }
    
    Interpreter& getInterpreter() { return interpreter_; }
    std::shared_ptr<ClawArray> getArray() { return array_; }
    
private:
    Interpreter& interpreter_;
    std::shared_ptr<ClawArray> array_;
};

} // namespace claw
