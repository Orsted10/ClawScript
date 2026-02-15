#include "vm.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "interpreter/errors.h"
#include "features/string_pool.h"
#include "features/callable.h"
#include "features/class.h"
#include "lexer/token.h"
#include "interpreter/interpreter.h"

namespace volt {

VM::VM()
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
      jit_(),
      globals_(std::make_shared<Environment>()),
      interpreter_(nullptr),
      globalVersion_(0),
      globalInlineCache_(),
      propertyInlineCache_(),
      instanceVersions_(),
      callInlineCache_() {
}

VM::VM(Interpreter& interpreter)
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
      jit_(),
      globals_(interpreter.getGlobals()),
      interpreter_(&interpreter),
      globalVersion_(0),
      globalInlineCache_(),
      propertyInlineCache_(),
      instanceVersions_(),
      callInlineCache_() {
}

InterpretResult VM::interpret(const Chunk& chunk) {
    chunk_ = &chunk;
    stackTop_ = stack_;
    frameCount_ = 0;
    openUpvalues_.clear();
    globalVersion_ = 0;
    globalInlineCache_.clear();
    propertyInlineCache_.clear();
    instanceVersions_.clear();
    callInlineCache_.clear();

    auto function = std::make_shared<VMFunction>();
    function->name = "<script>";
    function->arity = 0;
    function->upvalueCount = 0;
    function->chunk = std::make_shared<Chunk>(chunk);

    auto closure = std::make_shared<VMClosure>();
    closure->function = std::move(function);
    closure->upvalues.clear();
    vmClosureValue(closure);

    frames_[frameCount_++] = {closure.get(), closure->function->chunk->code().data(), stack_};
    ip_ = frames_[frameCount_ - 1].ip;
    return run();
}

InterpretResult VM::run() {
    CallFrame* frame = &frames_[frameCount_ - 1];
    Value* stackTop = stackTop_;
    (void)jit_;

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() frame->closure->function->chunk->constants()[READ_BYTE()]
#define READ_STRING_PTR() asStringPtr(READ_CONSTANT())
#define BINARY_OP(op) \
    do { \
        if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) { \
            stackTop_ = stackTop; \
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
            stackTop_ = stackTop; \
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
                const char* namePtr = READ_STRING_PTR();
                globals_->define(namePtr, *(--stackTop));
                globalVersion_++;
                break;
            }
            case OpCode::GetGlobal: {
                const uint8_t* cacheKey = frame->ip - 1;
                const char* namePtr = READ_STRING_PTR();
                auto cacheIt = globalInlineCache_.find(cacheKey);
                if (cacheIt != globalInlineCache_.end()) {
                    const auto& entry = cacheIt->second;
                    if (entry.name == namePtr && entry.version == globalVersion_) {
                        *stackTop++ = entry.value;
                        break;
                    }
                }
                if (!globals_->exists(namePtr)) {
                    stackTop_ = stackTop;
                    std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                Value value = globals_->get(namePtr);
                globalInlineCache_[cacheKey] = {namePtr, globalVersion_, value};
                *stackTop++ = value;
                break;
            }
            case OpCode::SetGlobal: {
                const char* namePtr = READ_STRING_PTR();
                if (!globals_->exists(namePtr)) {
                    stackTop_ = stackTop;
                    std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                globals_->assign(namePtr, stackTop[-1]);
                globalVersion_++;
                break;
            }
            case OpCode::GetLocal: {
                uint8_t slot = READ_BYTE();
                *stackTop++ = frame->slots[slot];
                break;
            }
            case OpCode::SetLocal: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = stackTop[-1];
                break;
            }
            case OpCode::GetUpvalue: {
                uint8_t slot = READ_BYTE();
                *stackTop++ = *frame->closure->upvalues[slot]->location;
                break;
            }
            case OpCode::SetUpvalue: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = stackTop[-1];
                break;
            }
            case OpCode::CloseUpvalue: {
                closeUpvalues(stackTop - 1);
                stackTop--;
                break;
            }

            case OpCode::Jump: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OpCode::JumpIfFalse: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(stackTop[-1])) frame->ip += offset;
                break;
            }
            case OpCode::Loop: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
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
                    stackTop_ = stackTop;
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
                    stackTop_ = stackTop;
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

            case OpCode::Call: {
                const uint8_t* cacheKey = frame->ip;
                uint8_t argCount = READ_BYTE();
                Value callee = stackTop[-1 - argCount];
                auto cacheIt = callInlineCache_.find(cacheKey);
                if (cacheIt != callInlineCache_.end()) {
                    auto& entry = cacheIt->second;
                    if (entry.callee == asObjectPtr(callee)) {
                        if (entry.kind == CallCacheKind::VMClosure && entry.closure) {
                            stackTop_ = stackTop;
                            if (!call(entry.closure, argCount)) {
                                stackTop_ = stackTop;
                                return InterpretResult::RuntimeError;
                            }
                            stackTop = stackTop_;
                            frame = &frames_[frameCount_ - 1];
                            break;
                        }
                        if (entry.kind == CallCacheKind::VMFunction && entry.closure) {
                            stackTop_ = stackTop;
                            if (!call(entry.closure, argCount)) {
                                stackTop_ = stackTop;
                                return InterpretResult::RuntimeError;
                            }
                            stackTop = stackTop_;
                            frame = &frames_[frameCount_ - 1];
                            break;
                        }
                    }
                }
                stackTop_ = stackTop;
                if (!callValue(callee, argCount)) {
                    stackTop_ = stackTop;
                    return InterpretResult::RuntimeError;
                }
                if (isObject(callee)) {
                    if (isVMClosure(callee)) {
                        auto closure = asVMClosurePtr(callee);
                        if (closure) {
                            callInlineCache_[cacheKey] = {asObjectPtr(callee), CallCacheKind::VMClosure, closure};
                        }
                    } else if (isVMFunction(callee)) {
                        auto function = asVMFunction(callee);
                        if (function && function->upvalueCount == 0) {
                            auto closure = std::make_shared<VMClosure>();
                            closure->function = function;
                            closure->upvalues.resize(function->upvalueCount);
                            vmClosureValue(closure);
                            callInlineCache_[cacheKey] = {asObjectPtr(callee), CallCacheKind::VMFunction, closure.get()};
                        }
                    }
                }
                stackTop = stackTop_;
                frame = &frames_[frameCount_ - 1];
                break;
            }

            case OpCode::Closure: {
                Value functionVal = READ_CONSTANT();
                auto function = asVMFunction(functionVal);
                if (!function) {
                    stackTop_ = stackTop;
                    std::cerr << "Expected function constant." << std::endl;
                    return InterpretResult::RuntimeError;
                }

                auto closure = std::make_shared<VMClosure>();
                closure->function = function;
                closure->upvalues.resize(function->upvalueCount);

                for (int i = 0; i < function->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }

                *stackTop++ = vmClosureValue(closure);
                break;
            }

            case OpCode::Return: {
                Value result = *(--stackTop);
                Value* frameSlots = frame->slots;
                closeUpvalues(frame->slots);
                frameCount_--;
                if (frameCount_ == 0) {
                    stackTop_ = stackTop;
                    return InterpretResult::Ok;
                }
                stackTop = frameSlots;
                *stackTop++ = result;
                stackTop_ = stackTop;
                frame = &frames_[frameCount_ - 1];
                break;
            }

            case OpCode::GetProperty: {
                const uint8_t* cacheKey = frame->ip - 1;
                const char* namePtr = READ_STRING_PTR();
                Value instanceVal = stackTop[-1];
                if (!isInstance(instanceVal)) {
                    stackTop_ = stackTop;
                    std::cerr << "Only instances have properties." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                auto instance = asInstance(instanceVal);
                const auto* instancePtr = instance.get();
                auto versionIt = instanceVersions_.find(instancePtr);
                uint64_t version = versionIt == instanceVersions_.end() ? 0 : versionIt->second;
                auto cacheIt = propertyInlineCache_.find(cacheKey);
                if (cacheIt != propertyInlineCache_.end()) {
                    const auto& entry = cacheIt->second;
                    if (entry.instance == instancePtr && entry.name == namePtr && entry.version == version) {
                        stackTop[-1] = entry.value;
                        break;
                    }
                }
                Token nameToken(TokenType::Identifier, namePtr, 0);
                Value value = instance->get(nameToken);
                propertyInlineCache_[cacheKey] = {instancePtr, namePtr, version, value};
                stackTop[-1] = value;
                break;
            }
            case OpCode::SetProperty: {
                const char* namePtr = READ_STRING_PTR();
                Value value = stackTop[-1];
                Value instanceVal = stackTop[-2];
                if (!isInstance(instanceVal)) {
                    stackTop_ = stackTop;
                    std::cerr << "Only instances have fields." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                auto instance = asInstance(instanceVal);
                Token nameToken(TokenType::Identifier, namePtr, 0);
                instance->set(nameToken, value);
                instanceVersions_[instance.get()]++;
                stackTop[-2] = value;
                stackTop--;
                break;
            }

            default:
                stackTop_ = stackTop;
                std::cerr << "Unknown opcode" << std::endl;
                return InterpretResult::RuntimeError;
        }
        stackTop_ = stackTop;
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING_PTR
#undef BINARY_OP
#undef COMPARE_OP
}

bool VM::call(VMClosure* closure, int argCount) {
    if (closure->function->arity != -1 && argCount != closure->function->arity) {
        std::cerr << "Expected " << closure->function->arity
                  << " arguments but got " << argCount << "." << std::endl;
        return false;
    }
    if (frameCount_ >= FRAMES_MAX) {
        std::cerr << "Stack overflow." << std::endl;
        return false;
    }
    CallFrame frame;
    frame.closure = closure;
    frame.ip = frame.closure->function->chunk->code().data();
    frame.slots = stackTop_ - argCount - 1;
    frames_[frameCount_++] = frame;
    return true;
}

bool VM::callValue(Value callee, int argCount) {
    if (isVMClosure(callee)) {
        auto closure = asVMClosurePtr(callee);
        if (!closure) {
            std::cerr << "Invalid closure." << std::endl;
            return false;
        }
        return call(closure, argCount);
    }
    if (isVMFunction(callee)) {
        auto closure = std::make_shared<VMClosure>();
        closure->function = asVMFunction(callee);
        closure->upvalues.resize(closure->function->upvalueCount);
        vmClosureValue(closure);
        return call(closure.get(), argCount);
    }
    if (!isCallable(callee) && !isClass(callee)) {
        std::cerr << "Can only call functions and classes." << std::endl;
        return false;
    }
    if (!interpreter_) {
        std::cerr << "VM Call opcode requires interpreter context." << std::endl;
        return false;
    }
    std::shared_ptr<Callable> function;
    if (isClass(callee)) {
        function = std::static_pointer_cast<Callable>(asClass(callee));
    } else {
        function = asCallable(callee);
    }
    if (function->arity() != -1 && argCount != function->arity()) {
        std::cerr << "Expected " << function->arity()
                  << " arguments but got " << argCount << "." << std::endl;
        return false;
    }
    std::vector<Value> arguments;
    arguments.reserve(argCount);
    for (int i = 0; i < argCount; i++) {
        arguments.push_back(stackTop_[-argCount + i]);
    }
    Value result = function->call(*interpreter_, arguments);
    stackTop_ -= (argCount + 1);
    *stackTop_++ = result;
    return true;
}

std::shared_ptr<VMUpvalue> VM::captureUpvalue(Value* local) {
    for (auto& upvalue : openUpvalues_) {
        if (upvalue->location == local) {
            return upvalue;
        }
    }
    auto created = std::make_shared<VMUpvalue>();
    created->location = local;
    openUpvalues_.push_back(created);
    return created;
}

void VM::closeUpvalues(Value* last) {
    for (auto& upvalue : openUpvalues_) {
        if (upvalue->location >= last) {
            upvalue->closed = *upvalue->location;
            upvalue->location = &upvalue->closed;
        }
    }
    openUpvalues_.erase(
        std::remove_if(openUpvalues_.begin(), openUpvalues_.end(),
                       [last](const std::shared_ptr<VMUpvalue>& upvalue) {
                           return upvalue->location == &upvalue->closed;
                       }),
        openUpvalues_.end());
}

} // namespace volt
