#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <array>
#include "chunk.h"
#include "interpreter/value.h"
#include "interpreter/environment.h"
#include "jit/jit.h"

namespace volt {

class Interpreter;

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
    explicit VM(Interpreter& interpreter);
    ~VM() = default;

    static constexpr int STACK_MAX = 256;
    static constexpr int FRAMES_MAX = 64;

    InterpretResult interpret(const Chunk& chunk);

private:
    struct CallFrame {
        VMClosure* closure;
        const uint8_t* ip;
        Value* slots;
    };
    struct GlobalInlineCache {
        const char* name;
        uint64_t version;
        Value value;
    };
    struct PropertyInlineCache {
        const VoltInstance* instance;
        const char* name;
        uint64_t version;
        Value value;
    };
    enum class CallCacheKind : uint8_t {
        None,
        VMClosure,
        VMFunction
    };
    struct CallInlineCache {
        void* callee = nullptr;
        CallCacheKind kind = CallCacheKind::None;
        VMClosure* closure = nullptr;
    };

    InterpretResult run();
    bool call(VMClosure* closure, int argCount);
    bool callValue(Value callee, int argCount);
    std::shared_ptr<VMUpvalue> captureUpvalue(Value* local);
    void closeUpvalues(Value* last);

    void push(Value value) {
        if (stackTop_ - stack_ >= STACK_MAX) {
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
    const uint8_t* ip_;
    Value stack_[STACK_MAX];
    Value* stackTop_;
    std::array<CallFrame, FRAMES_MAX> frames_;
    int frameCount_;
    std::vector<std::shared_ptr<VMUpvalue>> openUpvalues_;
    JitEngine jit_;
    
    std::shared_ptr<Environment> globals_;
    Interpreter* interpreter_;
    uint64_t globalVersion_;
    std::unordered_map<const uint8_t*, GlobalInlineCache> globalInlineCache_;
    std::unordered_map<const uint8_t*, PropertyInlineCache> propertyInlineCache_;
    std::unordered_map<const VoltInstance*, uint64_t> instanceVersions_;
    std::unordered_map<const uint8_t*, CallInlineCache> callInlineCache_;
};

} // namespace volt
