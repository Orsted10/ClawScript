#include "vm.h"
#include <iostream>
#include <cstdio>
#include "interpreter/errors.h"
#include "features/string_pool.h"

namespace volt {

VM::VM() : chunk_(nullptr), ip_(nullptr) {
    globals_ = std::make_shared<Environment>();
    stackTop_ = stack_;
}

InterpretResult VM::interpret(const Chunk& chunk) {
    chunk_ = &chunk;
    ip_ = chunk_->code().data();
    stackTop_ = stack_;
    return run();
}

InterpretResult VM::run() {
    // Local copies of hot variables for register allocation
    const uint8_t* ip = ip_;
    Value* stackTop = stackTop_;
    (void)jit_;

#define READ_BYTE() (*ip++)
#define READ_SHORT() \
    (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONSTANT() (*chunk_).constants()[READ_BYTE()]
#define READ_STRING() asString(READ_CONSTANT())
#define BINARY_OP(op) \
    do { \
        if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) { \
            ip_ = ip; stackTop_ = stackTop; \
            std::cerr << "Operands must be numbers." << std::endl; \
            return InterpretResult::RuntimeError; \
        } \
        double b = asNumber(*(--stackTop)); \
        double a = asNumber(*(--stackTop)); \
        *stackTop++ = numberToValue(a op b); \
    } while (false)
#define COMPARE_OP(op) \
    do { \
        if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) { \
            ip_ = ip; stackTop_ = stackTop; \
            std::cerr << "Operands must be numbers." << std::endl; \
            return InterpretResult::RuntimeError; \
        } \
        double b = asNumber(*(--stackTop)); \
        double a = asNumber(*(--stackTop)); \
        *stackTop++ = boolValue(a op b); \
    } while (false)

    for (;;) {
        OpCode instruction = static_cast<OpCode>(READ_BYTE());
        switch (instruction) {
            case OpCode::Constant: {
                Value constant = READ_CONSTANT();
                *stackTop++ = constant;
                break;
            }
            case OpCode::Nil: *stackTop++ = nilValue(); break;
            case OpCode::True: *stackTop++ = boolValue(true); break;
            case OpCode::False: *stackTop++ = boolValue(false); break;
            case OpCode::Pop: stackTop--; break;
            
            case OpCode::DefineGlobal: {
                std::string name = READ_STRING();
                globals_->define(name, *(--stackTop));
                break;
            }
            case OpCode::GetGlobal: {
                std::string name = READ_STRING();
                if (!globals_->exists(name)) {
                    ip_ = ip; stackTop_ = stackTop;
                    std::cerr << "Undefined variable '" << name << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                *stackTop++ = globals_->get(name);
                break;
            }
            case OpCode::SetGlobal: {
                std::string name = READ_STRING();
                if (!globals_->exists(name)) {
                    ip_ = ip; stackTop_ = stackTop;
                    std::cerr << "Undefined variable '" << name << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                globals_->assign(name, stackTop[-1]);
                break;
            }
            case OpCode::GetLocal: {
                uint8_t slot = READ_BYTE();
                *stackTop++ = stack_[slot];
                break;
            }
            case OpCode::SetLocal: {
                uint8_t slot = READ_BYTE();
                stack_[slot] = stackTop[-1];
                break;
            }

            case OpCode::Jump: {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }
            case OpCode::JumpIfFalse: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(stackTop[-1])) ip += offset;
                break;
            }
            case OpCode::Loop: {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }

            case OpCode::Add: {
                if (isString(stackTop[-1]) && isString(stackTop[-2])) {
                    std::string b = asString(*(--stackTop));
                    std::string a = asString(*(--stackTop));
                    auto sv = StringPool::intern(a + b);
                    *stackTop++ = stringValue(sv.data());
                } else if (isNumber(stackTop[-1]) && isNumber(stackTop[-2])) {
                    double b = asNumber(*(--stackTop));
                    double a = asNumber(*(--stackTop));
                    *stackTop++ = numberToValue(a + b);
                } else {
                    ip_ = ip; stackTop_ = stackTop;
                    std::cerr << "Operands must be two numbers or two strings." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::Subtract: BINARY_OP(-); break;
            case OpCode::Multiply: BINARY_OP(*); break;
            case OpCode::Divide:   BINARY_OP(/); break;
            
            case OpCode::Equal: {
                Value b = *(--stackTop);
                Value a = *(--stackTop);
                *stackTop++ = boolValue(isEqual(a, b));
                break;
            }
            case OpCode::Greater:  COMPARE_OP(>); break;
            case OpCode::Less:     COMPARE_OP(<); break;

            case OpCode::Not: {
                Value val = *(--stackTop);
                *stackTop++ = boolValue(isFalsey(val));
                break;
            }
            case OpCode::Negate: {
                if (!isNumber(stackTop[-1])) {
                    ip_ = ip; stackTop_ = stackTop;
                    std::cerr << "Operand must be a number." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                double val = asNumber(*(--stackTop));
                *stackTop++ = numberToValue(-val);
                break;
            }

            case OpCode::Print: {
                Value val = *(--stackTop);
                if (isNil(val)) std::cout << "nil" << std::endl;
                else if (isBool(val)) std::cout << (asBool(val) ? "true" : "false") << std::endl;
                else if (isNumber(val)) std::cout << asNumber(val) << std::endl;
                else if (isString(val)) std::cout << asString(val) << std::endl;
                break;
            }

            case OpCode::Return: {
                ip_ = ip; stackTop_ = stackTop;
                return InterpretResult::Ok;
            }

            default:
                ip_ = ip; stackTop_ = stackTop;
                std::cerr << "Unknown opcode" << std::endl;
                return InterpretResult::RuntimeError;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

} // namespace volt
