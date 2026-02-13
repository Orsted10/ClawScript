#include "vm.h"
#include <iostream>
#include <cstdio>
#include "interpreter/errors.h"

namespace volt {

VM::VM() : chunk_(nullptr), ip_(0) {
    globals_ = std::make_shared<Environment>();
}

InterpretResult VM::interpret(const Chunk& chunk) {
    chunk_ = &chunk;
    ip_ = 0;
    return run();
}

InterpretResult VM::run() {
#define READ_BYTE() (*chunk_).code()[ip_++]
#define READ_SHORT() \
    (ip_ += 2, (uint16_t)(((*chunk_).code()[ip_ - 2] << 8) | (*chunk_).code()[ip_ - 1]))
#define READ_CONSTANT() (*chunk_).constants()[READ_BYTE()]
#define READ_STRING() std::get<std::string>(READ_CONSTANT())
#define BINARY_OP(op) \
    do { \
        if (!isNumber(peek(0)) || !isNumber(peek(1))) { \
            std::cerr << "Operands must be numbers." << std::endl; \
            return InterpretResult::RuntimeError; \
        } \
        double b = std::get<double>(pop()); \
        double a = std::get<double>(pop()); \
        push(a op b); \
    } while (false)

    for (;;) {
        OpCode instruction = static_cast<OpCode>(READ_BYTE());
        switch (instruction) {
            case OpCode::Constant: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OpCode::Nil: push(nullptr); break;
            case OpCode::True: push(true); break;
            case OpCode::False: push(false); break;
            case OpCode::Pop: pop(); break;
            
            case OpCode::DefineGlobal: {
                std::string name = READ_STRING();
                globals_->define(name, pop());
                break;
            }
            case OpCode::GetGlobal: {
                std::string name = READ_STRING();
                if (!globals_->exists(name)) {
                    std::cerr << "Undefined variable '" << name << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                push(globals_->get(name));
                break;
            }
            case OpCode::SetGlobal: {
                std::string name = READ_STRING();
                if (!globals_->exists(name)) {
                    std::cerr << "Undefined variable '" << name << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                globals_->assign(name, peek(0));
                break;
            }
            case OpCode::GetLocal: {
                uint8_t slot = READ_BYTE();
                push(stack_[slot]);
                break;
            }
            case OpCode::SetLocal: {
                uint8_t slot = READ_BYTE();
                stack_[slot] = peek(0);
                break;
            }

            case OpCode::Jump: {
                uint16_t offset = READ_SHORT();
                ip_ += offset;
                break;
            }
            case OpCode::JumpIfFalse: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) ip_ += offset;
                break;
            }
            case OpCode::Loop: {
                uint16_t offset = READ_SHORT();
                ip_ -= offset;
                break;
            }

            case OpCode::Add: {
                if (isString(peek(0)) && isString(peek(1))) {
                    std::string b = std::get<std::string>(pop());
                    std::string a = std::get<std::string>(pop());
                    push(a + b);
                } else if (isNumber(peek(0)) && isNumber(peek(1))) {
                    double b = std::get<double>(pop());
                    double a = std::get<double>(pop());
                    push(a + b);
                } else {
                    std::cerr << "Operands must be two numbers or two strings." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::Subtract: BINARY_OP(-); break;
            case OpCode::Multiply: BINARY_OP(*); break;
            case OpCode::Divide:   BINARY_OP(/); break;
            
            case OpCode::Equal: {
                Value b = pop();
                Value a = pop();
                push(a == b);
                break;
            }
            case OpCode::Greater:  BINARY_OP(>); break;
            case OpCode::Less:     BINARY_OP(<); break;

            case OpCode::Not: push(isFalsey(pop())); break;
            case OpCode::Negate: {
                if (!isNumber(peek(0))) {
                    std::cerr << "Operand must be a number." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                push(-std::get<double>(pop()));
                break;
            }

            case OpCode::Print: {
                // For now, just print to stdout
                Value val = pop();
                // Simple print logic for now
                if (isNil(val)) std::cout << "nil" << std::endl;
                else if (isBool(val)) std::cout << (std::get<bool>(val) ? "true" : "false") << std::endl;
                else if (isNumber(val)) std::cout << std::get<double>(val) << std::endl;
                else if (isString(val)) std::cout << std::get<std::string>(val) << std::endl;
                break;
            }

            case OpCode::Return: {
                return InterpretResult::Ok;
            }

            default:
                std::cerr << "Unknown opcode" << std::endl;
                return InterpretResult::RuntimeError;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

} // namespace volt
