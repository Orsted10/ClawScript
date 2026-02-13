#pragma once
#include <vector>
#include <memory>
#include "chunk.h"
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace volt {

enum class InterpretResult {
    Ok,
    CompileError,
    RuntimeError
};

/**
 * @brief Stack-based Virtual Machine for executing VoltScript bytecode
 */
class VM {
public:
    VM();
    ~VM() = default;

    InterpretResult interpret(const Chunk& chunk);

private:
    InterpretResult run();

    // Stack operations
    void push(Value value) { stack_.push_back(value); }
    Value pop() {
        Value val = stack_.back();
        stack_.pop_back();
        return val;
    }
    Value peek(int distance = 0) const {
        return stack_[stack_.size() - 1 - distance];
    }

    bool isFalsey(Value value) const {
        if (isNil(value)) return true;
        if (isBool(value)) return !std::get<bool>(value);
        return false;
    }

    const Chunk* chunk_;
    uint32_t ip_; // Instruction pointer
    std::vector<Value> stack_;
    
    // Global variables are shared with the interpreter for now
    std::shared_ptr<Environment> globals_;
};

} // namespace volt
