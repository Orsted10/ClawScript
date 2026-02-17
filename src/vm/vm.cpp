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

RuntimeFlags gRuntimeFlags;

VM::VM()
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
#ifdef VOLT_ENABLE_JIT
      jit_(),
#endif
      globals_(std::make_shared<Environment>()),
      interpreter_(nullptr),
      globalVersion_(0),
      globalInlineCache_(),
      propertyInlineCache_(),
      instanceVersions_(),
      callInlineCache_(),
      functionHotness_(),
      loopHotness_()
#ifdef VOLT_ENABLE_JIT
      , jitConfig_()
#endif
{
#ifdef VOLT_ENABLE_JIT
    jit_.setConfig(gJitConfig);
    jitConfig_ = gJitConfig;
#endif
    gcRegisterVM(this);
}

VM::VM(Interpreter& interpreter)
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
#ifdef VOLT_ENABLE_JIT
      jit_(),
#endif
      globals_(interpreter.getGlobals()),
      interpreter_(&interpreter),
      globalVersion_(0),
      globalInlineCache_(),
      propertyInlineCache_(),
      instanceVersions_(),
      callInlineCache_(),
      functionHotness_(),
      loopHotness_()
#ifdef VOLT_ENABLE_JIT
      , jitConfig_()
#endif
{
#ifdef VOLT_ENABLE_JIT
    jit_.setConfig(gJitConfig);
    jitConfig_ = gJitConfig;
#endif
    gcRegisterVM(this);
}

VM::~VM() { gcUnregisterVM(this); }
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
#ifdef VOLT_ENABLE_JIT
    (void)jit_;
#endif

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
#ifdef VOLT_ENABLE_JIT
                auto& c = loopHotness_[frame->ip];
                c++;
                if (c.load() >= (jitConfig_.aggressive ? std::max(1u, jitConfig_.loopThreshold / 4) : jitConfig_.loopThreshold)) {
                    const uint8_t* header = frame->ip - offset;
                    if (!jit_.hasBaseline(frame->closure->function.get())) {
                        std::vector<JitEntry> entries;
                        entries.push_back({header, nullptr, JitTier::Baseline});
                        jit_.registerBaseline(frame->closure->function.get(), entries);
                    }
                    if (jit_.enterOSR(this, frame->closure->function.get(), header)) {
                        break;
                    }
                }
#endif
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
                size_t avail = static_cast<size_t>(stackTop - stack_);
                if (static_cast<size_t>(argCount + 1) > avail) {
                    stackTop_ = stackTop;
                    std::cerr << "Invalid call: argCount exceeds stack." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                Value callee = stackTop[-1 - argCount];
                if (!gRuntimeFlags.disableCallIC) {
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
                } else if (gRuntimeFlags.icDiagnostics) {
                    std::fprintf(stderr, "[IC] disabled path key=%p argc=%u callee=%p sp=%p ip=%p\n",
                                 (const void*)cacheKey, (unsigned)argCount, (void*)asObjectPtr(callee),
                                 (void*)stackTop, (const void*)frame->ip);
                }
                stackTop_ = stackTop;
                if (!callValue(callee, argCount)) {
                    stackTop_ = stackTop;
                    return InterpretResult::RuntimeError;
                }
                if (!gRuntimeFlags.disableCallIC) {
                    if (isObject(callee)) {
                        if (isVMClosure(callee)) {
                            auto closure = asVMClosurePtr(callee);
                            if (closure) {
                                callInlineCache_[cacheKey] = {asObjectPtr(callee), CallCacheKind::VMClosure, closure};
                                if (gRuntimeFlags.icDiagnostics) {
                                    std::fprintf(stderr, "[IC] cache store closure key=%p callee=%p closure=%p\n",
                                                 (const void*)cacheKey, (void*)asObjectPtr(callee), (void*)closure);
                                }
                            }
                        } else if (isVMFunction(callee)) {
                            auto function = asVMFunction(callee);
                            if (function && function->upvalueCount == 0) {
                                auto closure = std::make_shared<VMClosure>();
                                closure->function = function;
                                closure->upvalues.resize(function->upvalueCount);
                                vmClosureValue(closure);
                                callInlineCache_[cacheKey] = {asObjectPtr(callee), CallCacheKind::VMFunction, closure.get()};
                                if (gRuntimeFlags.icDiagnostics) {
                                    std::fprintf(stderr, "[IC] cache store function key=%p callee=%p closure=%p\n",
                                                 (const void*)cacheKey, (void*)asObjectPtr(callee), (void*)closure.get());
                                }
                            }
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
                gcEphemeralEscape(result);
                gcEphemeralFrameLeave();
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
                lastPropertySiteIp_ = cacheKey;
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
#ifndef VOLT_DISABLE_IC_DIAGNOSTICS
                if (propertyICMegamorphic_.count(cacheKey)) {
                    Token nameToken(TokenType::Identifier, namePtr, 0);
                    stackTop[-1] = instance->get(nameToken);
                    if (gRuntimeFlags.icDiagnostics) {
                        std::fprintf(stderr, "[IC] megamorphic GetProperty key=%p name=%p inst=%p\n",
                                     (const void*)cacheKey, (const void*)namePtr, (const void*)instancePtr);
                    }
                    break;
                }
#endif
                auto& entries = propertyInlineCache_[cacheKey];
                bool hit = false;
                for (auto& e : entries) {
                    if (e.instance == instancePtr && e.name == namePtr && e.version == version) {
                        stackTop[-1] = e.value;
                        hit = true;
                        break;
                    }
                }
                if (hit) break;
                Token nameToken(TokenType::Identifier, namePtr, 0);
                Value value = instance->get(nameToken);
                if (entries.size() >= 8) {
                    entries.erase(entries.begin());
                }
                entries.push_back({instancePtr, namePtr, version, value});
#ifndef VOLT_DISABLE_IC_DIAGNOSTICS
                auto& miss = propertyICMissCount_[cacheKey];
                miss++;
                if (miss > 16) {
                    propertyICMegamorphic_.insert(cacheKey);
                    if (gRuntimeFlags.icDiagnostics) {
                        std::fprintf(stderr, "[IC] promote megamorphic GetProperty key=%p misses=%u\n",
                                     (const void*)cacheKey, miss);
                    }
                } else if (gRuntimeFlags.icDiagnostics) {
                    std::fprintf(stderr, "[IC] property miss key=%p misses=%u\n", (const void*)cacheKey, miss);
                }
#endif
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
                gcEphemeralEscape(value);
                instanceVersions_[instance.get()]++;
                stackTop[-2] = value;
                stackTop--;
                break;
            }

            default:
                stackTop_ = stackTop;
                std::cerr << "Unknown opcode " << static_cast<unsigned>(instruction) << std::endl;
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

bool VM::osrEnter(const uint8_t* ip) {
    if (frameCount_ <= 0) return false;
    frames_[frameCount_ - 1].ip = ip;
    return true;
}

bool VM::call(VMClosure* closure, int argCount) {
    auto& fc = functionHotness_[closure->function.get()];
    fc++;
#ifdef VOLT_ENABLE_JIT
    if (fc.load() >= (jitConfig_.aggressive ? std::max(1u, jitConfig_.functionThreshold / 4) : jitConfig_.functionThreshold)) {
        if (!jit_.hasBaseline(closure->function.get())) {
            std::vector<JitEntry> entries;
            entries.push_back({closure->function->chunk->code().data(), nullptr, JitTier::Baseline});
            jit_.registerBaseline(closure->function.get(), entries);
        }
    }
#endif
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
    gcEphemeralFrameEnter();
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
    gcEphemeralFrameEnter();
    std::vector<Value> arguments;
    arguments.reserve(argCount);
    for (int i = 0; i < argCount; i++) {
        arguments.push_back(stackTop_[-argCount + i]);
    }
    Value result = function->call(*interpreter_, arguments);
    gcEphemeralFrameLeave();
    stackTop_ -= (argCount + 1);
    *stackTop_++ = result;
    return true;
}

std::shared_ptr<VMUpvalue> VM::captureUpvalue(Value* local) {
    for (auto& upvalue : openUpvalues_) {
        if (upvalue->location == local) {
            gcEphemeralEscape(*local);
            return upvalue;
        }
    }
    auto created = std::make_shared<VMUpvalue>();
    created->location = local;
    openUpvalues_.push_back(created);
    gcEphemeralEscape(*local);
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
namespace volt {
uint8_t VM::apiReadByte() { return *frames_[frameCount_ - 1].ip++; }
uint16_t VM::apiReadShort() {
    auto& ip = frames_[frameCount_ - 1].ip;
    uint16_t b1 = *ip++;
    uint16_t b2 = *ip++;
    return static_cast<uint16_t>((b1 << 8) | b2);
}
uint64_t VM::apiReadConstant() {
    auto& frame = frames_[frameCount_ - 1];
    uint8_t idx = apiReadByte();
    return frame.closure->function->chunk->constants()[idx];
}
const char* VM::apiReadStringPtr() { return asStringPtr(apiReadConstant()); }
void VM::apiSetIp(const uint8_t* ip) { frames_[frameCount_ - 1].ip = ip; }
const uint8_t* VM::apiGetIp() const { return frames_[frameCount_ - 1].ip; }
void VM::apiPush(Value v) { push(v); }
Value VM::apiPop() { return pop(); }
Value VM::apiPeek(int distance) const { return peek(distance); }
void VM::apiSetLocal(int slot, Value v) { frames_[frameCount_ - 1].slots[slot] = v; }
Value VM::apiGetLocal(int slot) { return frames_[frameCount_ - 1].slots[slot]; }
void VM::apiJump(uint16_t offset) { frames_[frameCount_ - 1].ip += offset; }
void VM::apiJumpIfFalse(uint16_t offset) {
    if (isFalsey(peek())) frames_[frameCount_ - 1].ip += offset;
}
void VM::apiLoop(uint16_t offset) { frames_[frameCount_ - 1].ip -= offset; }
bool VM::apiReturn() {
    auto& frame = frames_[frameCount_ - 1];
    Value result = pop();
    Value* frameSlots = frame.slots;
    closeUpvalues(frame.slots);
    gcEphemeralEscape(result);
    gcEphemeralFrameLeave();
    frameCount_--;
    if (frameCount_ == 0) {
        push(result);
        return true;
    }
    stackTop_ = frameSlots;
    push(result);
    return false;
}
bool VM::apiIsFalsey(Value v) const { return isFalsey(v); }
void VM::apiDefineGlobal(const char* name, Value v) {
    globals_->define(name, v);
    globalVersion_++;
    gcEphemeralEscape(v);
}
bool VM::apiGlobalExists(const char* name) const { return globals_->exists(name); }
Value VM::apiGlobalGet(const char* name) const { return globals_->get(name); }
void VM::apiGlobalAssign(const char* name, Value v) {
    globals_->assign(name, v);
    globalVersion_++;
    gcEphemeralEscape(v);
}
void VM::apiBumpGlobalVersion() { globalVersion_++; }
std::shared_ptr<VMUpvalue> VM::apiCaptureUpvalue(Value* local) { return captureUpvalue(local); }
Value* VM::apiCurrentSlots() { return frames_[frameCount_ - 1].slots; }
bool VM::apiCallValue(Value callee, int argCount) { return callValue(callee, argCount); }
VMClosure* VM::apiCurrentClosure() { return frames_[frameCount_ - 1].closure; }
void VM::apiCloseTopUpvalue() { closeUpvalues(stackTop_ - 1); pop(); }
int VM::apiTryGetGlobalCached(const char* name, const uint8_t* siteIp, Value* out) {
    auto it = globalInlineCache_.find(siteIp);
    if (it == globalInlineCache_.end()) return 0;
    const auto& entry = it->second;
    if (entry.name == name && entry.version == globalVersion_) {
        *out = entry.value;
        return 1;
    }
    return 0;
}
int VM::apiTryGetPropertyCached(Value instanceVal, const char* name, const uint8_t* siteIp, Value* out) {
    if (!isInstance(instanceVal)) return 0;
    auto inst = asInstance(instanceVal);
    const auto* ptr = inst.get();
    auto vit = instanceVersions_.find(ptr);
    uint64_t ver = vit == instanceVersions_.end() ? 0 : vit->second;
    auto pit = propertyInlineCache_.find(siteIp);
    if (pit == propertyInlineCache_.end()) return 0;
    for (auto& e : pit->second) {
        if (e.instance == ptr && e.name == name && e.version == ver) {
            *out = e.value;
            return 1;
        }
    }
    return 0;
}
int VM::apiTryCallCached(const uint8_t* siteIp, uint8_t argCount) {
    auto it = callInlineCache_.find(siteIp);
    if (it == callInlineCache_.end()) return 0;
    auto& entry = it->second;
    Value callee = apiPeek(argCount);
    if (entry.callee == asObjectPtr(callee) && entry.closure) {
        if (!call(entry.closure, argCount)) return 0;
        return 1;
    }
    return 0;
}
uint32_t VM::apiGetFunctionHotness(const VMFunction* fn) {
    auto it = functionHotness_.find(fn);
    if (it == functionHotness_.end()) return 0;
    return it->second.load();
}
uint32_t VM::apiGetLoopHotness(const uint8_t* ip) {
    auto it = loopHotness_.find(ip);
    if (it == loopHotness_.end()) return 0;
    return it->second.load();
}
#ifdef VOLT_ENABLE_JIT
bool VM::apiHasBaseline(const VMFunction* fn) {
    return jit_.hasBaseline(fn);
}
#endif
} // namespace volt

namespace volt {
void VM::forEachRoot(const std::function<void(Value)>& fn) const {
    for (const Value* p = stack_; p < stackTop_; ++p) fn(*p);
    for (int i = 0; i < frameCount_; ++i) {
        auto& fr = frames_[i];
        if (fr.slots) {
            Value* s = fr.slots;
            while (s < stackTop_) { fn(*s); ++s; }
        }
        if (fr.closure) {
            for (auto& up : fr.closure->upvalues) {
                if (up->location) fn(*up->location);
                fn(up->closed);
            }
        }
    }
    if (globals_) {
        globals_->forEachValue(fn);
    }
}
} // namespace volt

extern "C" uint8_t volt_vm_read_byte(volt::VM* vm) { return vm->apiReadByte(); }
extern "C" uint16_t volt_vm_read_short(volt::VM* vm) { return vm->apiReadShort(); }
extern "C" uint64_t volt_vm_read_constant(volt::VM* vm) { return vm->apiReadConstant(); }
extern "C" const char* volt_vm_read_string_ptr(volt::VM* vm) { return vm->apiReadStringPtr(); }
extern "C" void volt_vm_set_ip(volt::VM* vm, const uint8_t* ip) { vm->apiSetIp(ip); }
extern "C" const uint8_t* volt_vm_get_ip(volt::VM* vm) { return vm->apiGetIp(); }
extern "C" void volt_vm_push(volt::VM* vm, uint64_t v) { vm->apiPush(v); }
extern "C" uint64_t volt_vm_pop(volt::VM* vm) { return vm->apiPop(); }
extern "C" uint64_t volt_vm_peek(volt::VM* vm, int distance) { return vm->apiPeek(distance); }
extern "C" void volt_vm_set_local(volt::VM* vm, int slot, uint64_t v) { vm->apiSetLocal(slot, v); }
extern "C" uint64_t volt_vm_get_local(volt::VM* vm, int slot) { return vm->apiGetLocal(slot); }
extern "C" void volt_vm_jump(volt::VM* vm, uint16_t offset) { vm->apiJump(offset); }
extern "C" void volt_vm_jump_if_false(volt::VM* vm, uint16_t offset) { vm->apiJumpIfFalse(offset); }
extern "C" void volt_vm_loop(volt::VM* vm, uint16_t offset) { vm->apiLoop(offset); }
extern "C" void volt_vm_binary_add(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::numberToValue(a + b));
}
extern "C" void volt_vm_binary_sub(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::numberToValue(a - b));
}
extern "C" void volt_vm_binary_mul(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::numberToValue(a * b));
}
extern "C" void volt_vm_binary_div(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::numberToValue(a / b));
}
extern "C" void volt_vm_compare_eq(volt::VM* vm) {
    auto b = vm->apiPop();
    auto a = vm->apiPop();
    vm->apiPush(volt::boolValue(volt::isEqual(a, b)));
}
extern "C" void volt_vm_compare_gt(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::boolValue(a > b));
}
extern "C" void volt_vm_compare_lt(volt::VM* vm) {
    auto b = volt::asNumber(vm->apiPop());
    auto a = volt::asNumber(vm->apiPop());
    vm->apiPush(volt::boolValue(a < b));
}
extern "C" void volt_vm_unary_not(volt::VM* vm) {
    vm->apiPush(volt::boolValue(vm->apiIsFalsey(vm->apiPop())));
}
extern "C" void volt_vm_unary_negate(volt::VM* vm) {
    vm->apiPush(volt::numberToValue(-volt::asNumber(vm->apiPop())));
}
extern "C" void volt_vm_print(volt::VM* vm) {
    auto v = vm->apiPop();
    if (volt::isNil(v)) std::cout << "nil" << std::endl;
    else if (volt::isBool(v)) std::cout << (volt::asBool(v) ? "true" : "false") << std::endl;
    else if (volt::isNumber(v)) std::cout << volt::asNumber(v) << std::endl;
    else if (volt::isString(v)) std::cout << volt::asString(v) << std::endl;
    else std::cout << volt::valueToString(v) << std::endl;
}
extern "C" void volt_vm_get_global(volt::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    if (!vm->apiGlobalExists(namePtr)) {
        std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
        vm->apiPush(volt::nilValue());
        return;
    }
    volt::Value value = vm->apiGlobalGet(namePtr);
    vm->apiPush(value);
}
extern "C" void volt_vm_define_global(volt::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    auto value = vm->apiPop();
    vm->apiDefineGlobal(namePtr, value);
}
extern "C" void volt_vm_set_global(volt::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    if (!vm->apiGlobalExists(namePtr)) {
        std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
        return;
    }
    vm->apiGlobalAssign(namePtr, vm->apiPeek(0));
}
extern "C" int volt_vm_try_get_global_cached(volt::VM* vm, const char* namePtr, const uint8_t* siteIp, uint64_t* out) {
    volt::Value v;
    int hit = vm->apiTryGetGlobalCached(namePtr, siteIp, &v);
    if (hit && out) *out = v;
    return hit;
}
extern "C" void volt_vm_call(volt::VM* vm) {
    uint8_t argCount = vm->apiReadByte();
    volt::Value callee = vm->apiPeek(argCount);
    vm->apiCallValue(callee, argCount);
}
extern "C" void volt_vm_closure(volt::VM* vm) {
    volt::Value functionVal = vm->apiReadConstant();
    auto function = volt::asVMFunction(functionVal);
    if (!function) {
        std::cerr << "Expected function constant." << std::endl;
        return;
    }
    auto closure = std::make_shared<volt::VMClosure>();
    closure->function = function;
    closure->upvalues.resize(function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        uint8_t isLocal = vm->apiReadByte();
        uint8_t index = vm->apiReadByte();
        if (isLocal) {
            closure->upvalues[i] = vm->apiCaptureUpvalue(vm->apiCurrentSlots() + index);
        } else {
            closure->upvalues[i] = vm->apiCurrentClosure()->upvalues[index];
        }
    }
    vm->apiPush(volt::vmClosureValue(closure));
}
extern "C" void volt_vm_get_upvalue(volt::VM* vm) {
    uint8_t slot = vm->apiReadByte();
    vm->apiPush(*vm->apiCurrentClosure()->upvalues[slot]->location);
}
extern "C" void volt_vm_set_upvalue(volt::VM* vm) {
    uint8_t slot = vm->apiReadByte();
    *vm->apiCurrentClosure()->upvalues[slot]->location = vm->apiPeek(0);
}
extern "C" void volt_vm_close_upvalue(volt::VM* vm) {
    vm->apiCloseTopUpvalue();
}
extern "C" int volt_vm_osr_enter(volt::VM* vm, const uint8_t* ip) {
    return vm->osrEnter(ip) ? 1 : 0;
}
extern "C" bool volt_vm_return(volt::VM* vm) { return vm->apiReturn(); }
extern "C" void volt_vm_get_property(volt::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    volt::Value instanceVal = vm->apiPeek();
    if (!volt::isInstance(instanceVal)) {
        std::cerr << "Only instances have properties." << std::endl;
        return;
    }
    auto instance = volt::asInstance(instanceVal);
    volt::Token nameToken(volt::TokenType::Identifier, namePtr, 0);
    volt::Value value = instance->get(nameToken);
    vm->apiPop();
    vm->apiPush(value);
}
extern "C" int volt_vm_try_get_property_cached(volt::VM* vm, uint64_t instanceVal, const char* namePtr, const uint8_t* siteIp, uint64_t* out) {
    volt::Value v;
    int hit = vm->apiTryGetPropertyCached(instanceVal, namePtr, siteIp, &v);
    if (hit && out) *out = v;
    return hit;
}
extern "C" int volt_vm_try_call_cached(volt::VM* vm, const uint8_t* siteIp, uint8_t argCount) {
    return vm->apiTryCallCached(siteIp, argCount);
}
extern "C" void volt_vm_set_property(volt::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    volt::Value value = vm->apiPeek(0);
    volt::Value instanceVal = vm->apiPeek(1);
    if (!volt::isInstance(instanceVal)) {
        std::cerr << "Only instances have fields." << std::endl;
        return;
    }
    auto instance = volt::asInstance(instanceVal);
    volt::Token nameToken(volt::TokenType::Identifier, namePtr, 0);
    instance->set(nameToken, value);
    vm->apiPop();
    vm->apiPop();
    vm->apiPush(value);
}
