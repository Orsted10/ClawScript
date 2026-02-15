#pragma once
#include <vector>
#include <memory>
#include "chunk.h"
#include "interpreter/value.h"
#include "interpreter/environment.h"
#include "jit/jit.h"

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

    static constexpr int STACK_MAX = 256;

    InterpretResult interpret(const Chunk& chunk);

private:
    InterpretResult run();

    // Stack operations
    void push(Value value) {
        if (stackTop_ - stack_ >= STACK_MAX) {
            // Handle stack overflow
            return;
        }
        *stackTop_++ = value;
    }
    Value pop() {
        return *(--stackTop_);
    }
    Value peek(int distance = 0) const {
        return stackTop_[-1 - distance];
    }

    bool isFalsey(Value value) const {
        if (isNil(value)) return true;
        if (isBool(value)) return !asBool(value);
        return false;
    }

    const Chunk* chunk_;
    const uint8_t* ip_; // Instruction pointer (raw pointer for speed)
    Value stack_[STACK_MAX];
    Value* stackTop_;
    JitEngine jit_;
    
    // Global variables are shared with the interpreter for now
    std::shared_ptr<Environment> globals_;
};

} // namespace volt
