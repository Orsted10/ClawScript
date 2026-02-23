#include "vm.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <vector>
#include <algorithm>
#include "interpreter/errors.h"
#include "features/string_pool.h"
#include "features/callable.h"
#include "features/class.h"
#include "features/array.h"
#include "features/hashmap.h"
#include "lexer/token.h"
#include "interpreter/interpreter.h"

namespace claw {

RuntimeFlags gRuntimeFlags;

VM::VM()
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
#ifdef CLAW_ENABLE_JIT
      jit_(),
#endif
      globals_(nullptr),
      interpreter_(nullptr),
      globalVersion_(0),
      globalInlineCache_(),
      propertyInlineCache_(),
      instanceVersions_(),
      callInlineCache_(),
      functionHotness_(),
      loopHotness_()
#ifdef CLAW_ENABLE_JIT
      , jitConfig_()
#endif
{
#ifdef CLAW_ENABLE_JIT
    jit_.setConfig(gJitConfig);
    jitConfig_ = gJitConfig;
#endif
    ownedInterpreter_ = std::make_unique<Interpreter>();
    interpreter_ = ownedInterpreter_.get();
    globals_ = interpreter_->getGlobals();
    gcRegisterVM(this);
}

VM::VM(Interpreter& interpreter)
    : chunk_(nullptr),
      ip_(nullptr),
      stackTop_(stack_),
      frames_(),
      frameCount_(0),
      openUpvalues_(),
#ifdef CLAW_ENABLE_JIT
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
#ifdef CLAW_ENABLE_JIT
      , jitConfig_()
#endif
{
#ifdef CLAW_ENABLE_JIT
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
#ifdef CLAW_ENABLE_JIT
    (void)jit_;
#endif
    static std::chrono::steady_clock::time_point lastCheck = std::chrono::steady_clock::now();
    static uint64_t lastAlloc = 0;

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
        if (gRuntimeFlags.idsEnabled) {
            if (frameCount_ > gRuntimeFlags.idsStackMax) {
                stackTop_ = stackTop;
                std::cerr << "Stack depth anomaly detected." << std::endl;
                return InterpretResult::RuntimeError;
            }
            auto now = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheck).count();
            if (dt >= 200) {
                uint64_t cur = gcGetYoungAllocations();
                uint64_t diff = cur >= lastAlloc ? (cur - lastAlloc) : 0;
                uint64_t rate = (uint64_t)((double)diff * 1000.0 / (double)dt);
                lastAlloc = cur;
                lastCheck = now;
                if (gRuntimeFlags.idsAllocRateMax && rate > gRuntimeFlags.idsAllocRateMax) {
                    stackTop_ = stackTop;
                    std::cerr << "Allocation rate anomaly detected." << std::endl;
                    return InterpretResult::RuntimeError;
                }
            }
        }
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
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[GetLocal] slot=" << (int)slot << " val=" << valueToString(frame->slots[slot]) << std::endl;
                }
                break;
            }
            case OpCode::SetLocal: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = stackTop[-1];
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[SetLocal] slot=" << (int)slot << " val=" << valueToString(stackTop[-1]) << std::endl;
                }
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
#ifdef CLAW_ENABLE_JIT
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
                Value vb = *(--stackTop);
                Value va = *(--stackTop);
                if (isString(va) && isString(vb)) {
                    auto sv = StringPool::intern(asString(va) + asString(vb));
                    *stackTop++ = stringValue(sv.data());
                } else if (isNumber(va) && isNumber(vb)) {
                    *stackTop++ = numberToValue(asNumber(va) + asNumber(vb));
                } else if (isString(va) && isNumber(vb)) {
                    auto sv = StringPool::intern(asString(va) + valueToString(vb));
                    *stackTop++ = stringValue(sv.data());
                } else if (isNumber(va) && isString(vb)) {
                    auto sv = StringPool::intern(valueToString(va) + asString(vb));
                    *stackTop++ = stringValue(sv.data());
                } else {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers or strings (supported: string+string, number+number, string+number, number+string)." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::Subtract: BINARY_OP(-); break;
            case OpCode::Multiply: BINARY_OP(*); break;
            // Override Divide to handle divide-by-zero with a clear error
            case OpCode::Divide: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                double b = asNumber(*(--stackTop));
                double a = asNumber(*(--stackTop));
                if (b == 0.0) {
                    stackTop_ = stackTop;
                    std::cerr << "Division by zero." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                *stackTop++ = numberToValue(a / b);
                break;
            }
            case OpCode::BitAnd: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers for bitwise AND." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                uint64_t b = static_cast<uint64_t>(asNumber(*(--stackTop)));
                uint64_t a = static_cast<uint64_t>(asNumber(*(--stackTop)));
                *stackTop++ = numberToValue(static_cast<double>(a & b));
                break;
            }
            case OpCode::BitOr: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers for bitwise OR." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                uint64_t b = static_cast<uint64_t>(asNumber(*(--stackTop)));
                uint64_t a = static_cast<uint64_t>(asNumber(*(--stackTop)));
                *stackTop++ = numberToValue(static_cast<double>(a | b));
                break;
            }
            case OpCode::BitXor: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers for bitwise XOR." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                uint64_t b = static_cast<uint64_t>(asNumber(*(--stackTop)));
                uint64_t a = static_cast<uint64_t>(asNumber(*(--stackTop)));
                *stackTop++ = numberToValue(static_cast<double>(a ^ b));
                break;
            }
            case OpCode::ShiftLeft: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers for shift left." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                double bDouble = asNumber(*(--stackTop));
                if (gRuntimeFlags.icDiagnostics) {
                    std::fprintf(stderr, "[ShiftLeft] count=%f\n", bDouble);
                }
                uint64_t a = static_cast<uint64_t>(asNumber(*(--stackTop)));
                int64_t bSigned = static_cast<int64_t>(bDouble);
                if (bDouble < 0.0 || bSigned < 0) {
                    stackTop_ = stackTop;
                    std::cerr << "Shift count must be non-negative." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                uint64_t b = static_cast<uint64_t>(bDouble);
                if (b > 0x7FFFFFFFFFFFFFFFULL) {
                    stackTop_ = stackTop;
                    std::cerr << "Shift count must be non-negative." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                int sh = static_cast<int>(b) & 63;
                *stackTop++ = numberToValue(static_cast<double>(static_cast<uint64_t>(a) << sh));
                break;
            }
            case OpCode::ShiftRight: {
                if (!isNumber(stackTop[-1]) || !isNumber(stackTop[-2])) {
                    stackTop_ = stackTop;
                    std::cerr << "Operands must be numbers for shift right." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                double bDouble = asNumber(*(--stackTop));
                if (gRuntimeFlags.icDiagnostics) {
                    std::fprintf(stderr, "[ShiftRight] count=%f\n", bDouble);
                }
                uint64_t a = static_cast<uint64_t>(asNumber(*(--stackTop)));
                int64_t bSigned = static_cast<int64_t>(bDouble);
                if (bDouble < 0.0 || bSigned < 0) {
                    stackTop_ = stackTop;
                    std::cerr << "Shift count must be non-negative." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                uint64_t b = static_cast<uint64_t>(bDouble);
                if (b > 0x7FFFFFFFFFFFFFFFULL) {
                    stackTop_ = stackTop;
                    std::cerr << "Shift count must be non-negative." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                int sh = static_cast<int>(b) & 63;
                *stackTop++ = numberToValue(static_cast<double>(static_cast<uint64_t>(a) >> sh));
                break;
            }
            
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
                std::cout << valueToString(val) << std::endl;
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
#ifndef CLAW_DISABLE_IC_DIAGNOSTICS
                if (propertyICMegamorphic_.count(cacheKey)) {
                    Token nameToken(TokenType::Identifier, namePtr, 0);
                    if (!instance->has(nameToken)) {
                        auto method = instance->getClass()->findMethod(std::string(namePtr));
                        if (!method) {
                            stackTop_ = stackTop;
                            std::cerr << "Undefined property '" << namePtr << "'." << std::endl;
                            return InterpretResult::RuntimeError;
                        }
                    }
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
                if (!instance->has(nameToken)) {
                    auto method = instance->getClass()->findMethod(std::string(namePtr));
                    if (!method) {
                        stackTop_ = stackTop;
                        std::cerr << "Undefined property '" << namePtr << "'." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                }
                Value value = instance->get(nameToken);
                if (entries.size() >= 8) {
                    entries.erase(entries.begin());
                }
                entries.push_back({instancePtr, namePtr, version, value});
#ifndef CLAW_DISABLE_IC_DIAGNOSTICS
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
            case OpCode::GetIndex: {
                Value index = *(--stackTop);
                Value object = *(--stackTop);
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[StackBeforeGetIndex] obj=" << valueToString(object)
                              << " idx=" << valueToString(index) << std::endl;
                }
                if (isArray(object)) {
                    if (!isNumber(index)) {
                        stackTop_ = stackTop;
                        std::cerr << "Array index must be a number." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    auto array = asArray(object);
                    int idx = static_cast<int>(asNumber(index));
                    if (idx < 0 || idx >= array->length()) {
                        stackTop_ = stackTop;
                        std::cerr << "Index " << idx << " out of bounds [0, " << (array->length() - 1) << "]." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    Value v = array->get(static_cast<size_t>(idx));
                    gcEphemeralEscape(v);
                    *stackTop++ = v;
                    if (gRuntimeFlags.icDiagnostics) {
                        std::cerr << "[GetIndexResult] " << valueToString(v) << std::endl;
                    }
                    break;
                }
                if (isHashMap(object)) {
                    auto map = asHashMap(object);
                    std::string key;
                    if (isString(index)) {
                        key = asString(index);
                    } else if (isNumber(index)) {
                        double num = asNumber(index);
                        if (num == static_cast<long long>(num)) {
                            key = std::to_string(static_cast<long long>(num));
                        } else {
                            key = std::to_string(num);
                            key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                            key.erase(key.find_last_not_of('.') + 1, std::string::npos);
                        }
                    } else if (isNil(index)) {
                        key = "nil";
                    } else if (isBool(index)) {
                        key = asBool(index) ? "true" : "false";
                    } else {
                        stackTop_ = stackTop;
                        std::cerr << "Hash map index must be string, number, boolean, or nil." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    {
                        Value v = map->get(key);
                        gcEphemeralEscape(v);
                        *stackTop++ = v;
                    }
                    break;
                }
                stackTop_ = stackTop;
                std::cerr << "Can only index arrays and hash maps." << std::endl;
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[IndexDebug] objectTag=" << std::hex << tagBits(object)
                              << " isObj=" << (isObject(object) ? 1 : 0)
                              << " isArr=" << (isArray(object) ? 1 : 0)
                              << " isMap=" << (isHashMap(object) ? 1 : 0)
                              << " objStr=" << valueToString(object) << std::endl;
                }
                return InterpretResult::RuntimeError;
            }
            case OpCode::SetIndex: {
                Value value = *(--stackTop);
                Value index = *(--stackTop);
                Value object = *(--stackTop);
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[BeforeSetIndex] obj=" << valueToString(object)
                              << " idx=" << valueToString(index)
                              << " val=" << valueToString(value) << std::endl;
                }
                if (isArray(object)) {
                    if (!isNumber(index)) {
                        stackTop_ = stackTop;
                        std::cerr << "Array index must be a number." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    auto array = asArray(object);
                    int idx = static_cast<int>(asNumber(index));
                    if (idx < 0 || idx >= array->length()) {
                        stackTop_ = stackTop;
                        std::cerr << "Index " << idx << " out of bounds [0, " << (array->length() - 1) << "]." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    array->set(static_cast<size_t>(idx), value);
                    gcEphemeralEscape(value);
                    *stackTop++ = value;
                    break;
                }
                if (isHashMap(object)) {
                    auto map = asHashMap(object);
                    std::string key;
                    if (isString(index)) {
                        key = asString(index);
                    } else if (isNumber(index)) {
                        double num = asNumber(index);
                        if (num == static_cast<long long>(num)) {
                            key = std::to_string(static_cast<long long>(num));
                        } else {
                            key = std::to_string(num);
                            key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                            key.erase(key.find_last_not_of('.') + 1, std::string::npos);
                        }
                    } else if (isNil(index)) {
                        key = "nil";
                    } else if (isBool(index)) {
                        key = asBool(index) ? "true" : "false";
                    } else {
                        stackTop_ = stackTop;
                        std::cerr << "Hash map index must be string, number, boolean, or nil." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    map->set(key, value);
                    gcEphemeralEscape(value);
                    *stackTop++ = value;
                    break;
                }
                stackTop_ = stackTop;
                std::cerr << "Can only index arrays and hash maps." << std::endl;
                if (gRuntimeFlags.icDiagnostics) {
                    std::cerr << "[IndexDebug] objectTag=" << std::hex << tagBits(object)
                              << " isObj=" << (isObject(object) ? 1 : 0)
                              << " isArr=" << (isArray(object) ? 1 : 0)
                              << " isMap=" << (isHashMap(object) ? 1 : 0)
                              << " objStr=" << valueToString(object) << std::endl;
                }
                return InterpretResult::RuntimeError;
            }
            case OpCode::EnsureIndexDefault: {
                uint8_t opTag = READ_BYTE(); // 0:Add,1:Sub,2:Mul,3:Div,4:And,5:Or,6:Xor,7:Shl,8:Shr
                Value rhs = stackTop[-1];
                Value index = stackTop[-2];
                Value object = stackTop[-3];
                if (isHashMap(object)) {
                    auto map = asHashMap(object);
                    std::string key;
                    if (isString(index)) {
                        key = asString(index);
                    } else if (isNumber(index)) {
                        double num = asNumber(index);
                        if (num == static_cast<long long>(num)) {
                            key = std::to_string(static_cast<long long>(num));
                        } else {
                            key = std::to_string(num);
                            key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                            key.erase(key.find_last_not_of('.') + 1, std::string::npos);
                        }
                    } else if (isNil(index)) {
                        key = "nil";
                    } else if (isBool(index)) {
                        key = asBool(index) ? "true" : "false";
                    } else {
                        stackTop_ = stackTop;
                        std::cerr << "Hash map index must be string, number, boolean, or nil." << std::endl;
                        return InterpretResult::RuntimeError;
                    }
                    Value defaultVal = numberToValue(0.0);
                    if (opTag == 0) { // Add
                        if (isString(rhs)) {
                            auto sv = StringPool::intern(std::string());
                            defaultVal = stringValue(sv.data());
                        } else {
                            defaultVal = numberToValue(0.0);
                        }
                    } else {
                        defaultVal = numberToValue(0.0);
                    }
                    map->ensureDefault(key, defaultVal);
                }
                // Arrays: do nothing; regular semantics apply
                break;
            }
            case OpCode::EnsurePropertyDefault: {
                const char* namePtr = READ_STRING_PTR();
                uint8_t opTag = READ_BYTE();
                Value rhs = stackTop[-1];
                Value object = stackTop[-2];
                if (!isInstance(object)) {
                    stackTop_ = stackTop;
                    std::cerr << "Only instances have fields." << std::endl;
                    return InterpretResult::RuntimeError;
                }
                auto instance = asInstance(object);
                Token nameToken(TokenType::Identifier, namePtr, 0);
                if (!instance->has(nameToken)) {
                    Value defaultVal = numberToValue(0.0);
                    if (opTag == 0) {
                        if (isString(rhs)) {
                            auto sv = StringPool::intern(std::string());
                            defaultVal = stringValue(sv.data());
                        } else {
                            defaultVal = numberToValue(0.0);
                        }
                    } else {
                        defaultVal = numberToValue(0.0);
                    }
                    instance->set(nameToken, defaultVal);
                    gcEphemeralEscapeDeep(defaultVal);
                }
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
#ifdef CLAW_ENABLE_JIT
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
    gcEphemeralEscapeDeep(result);
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

} // namespace claw
namespace claw {
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
    gcEphemeralEscapeDeep(result);
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
#ifdef CLAW_ENABLE_JIT
bool VM::apiHasBaseline(const VMFunction* fn) {
    return jit_.hasBaseline(fn);
}
#endif
} // namespace claw

namespace claw {
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
} // namespace claw

extern "C" uint8_t claw_vm_read_byte(claw::VM* vm) { return vm->apiReadByte(); }
extern "C" uint16_t claw_vm_read_short(claw::VM* vm) { return vm->apiReadShort(); }
extern "C" uint64_t claw_vm_read_constant(claw::VM* vm) { return vm->apiReadConstant(); }
extern "C" const char* claw_vm_read_string_ptr(claw::VM* vm) { return vm->apiReadStringPtr(); }
extern "C" void claw_vm_set_ip(claw::VM* vm, const uint8_t* ip) { vm->apiSetIp(ip); }
extern "C" const uint8_t* claw_vm_get_ip(claw::VM* vm) { return vm->apiGetIp(); }
extern "C" void claw_vm_push(claw::VM* vm, uint64_t v) { vm->apiPush(v); }
extern "C" uint64_t claw_vm_pop(claw::VM* vm) { return vm->apiPop(); }
extern "C" uint64_t claw_vm_peek(claw::VM* vm, int distance) { return vm->apiPeek(distance); }
extern "C" void claw_vm_set_local(claw::VM* vm, int slot, uint64_t v) { vm->apiSetLocal(slot, v); }
extern "C" uint64_t claw_vm_get_local(claw::VM* vm, int slot) { return vm->apiGetLocal(slot); }
extern "C" void claw_vm_jump(claw::VM* vm, uint16_t offset) { vm->apiJump(offset); }
extern "C" void claw_vm_jump_if_false(claw::VM* vm, uint16_t offset) { vm->apiJumpIfFalse(offset); }
extern "C" void claw_vm_loop(claw::VM* vm, uint16_t offset) { vm->apiLoop(offset); }
extern "C" void claw_vm_binary_add(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::numberToValue(a + b));
}
extern "C" void claw_vm_binary_sub(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::numberToValue(a - b));
}
extern "C" void claw_vm_binary_mul(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::numberToValue(a * b));
}
extern "C" void claw_vm_binary_div(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::numberToValue(a / b));
}
extern "C" void claw_vm_compare_eq(claw::VM* vm) {
    auto b = vm->apiPop();
    auto a = vm->apiPop();
    vm->apiPush(claw::boolValue(claw::isEqual(a, b)));
}
extern "C" void claw_vm_compare_gt(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::boolValue(a > b));
}
extern "C" void claw_vm_compare_lt(claw::VM* vm) {
    auto b = claw::asNumber(vm->apiPop());
    auto a = claw::asNumber(vm->apiPop());
    vm->apiPush(claw::boolValue(a < b));
}
extern "C" void claw_vm_unary_not(claw::VM* vm) {
    vm->apiPush(claw::boolValue(vm->apiIsFalsey(vm->apiPop())));
}
extern "C" void claw_vm_unary_negate(claw::VM* vm) {
    vm->apiPush(claw::numberToValue(-claw::asNumber(vm->apiPop())));
}
extern "C" void claw_vm_print(claw::VM* vm) {
    auto v = vm->apiPop();
    if (claw::isNil(v)) std::cout << "nil" << std::endl;
    else if (claw::isBool(v)) std::cout << (claw::asBool(v) ? "true" : "false") << std::endl;
    else if (claw::isNumber(v)) std::cout << claw::asNumber(v) << std::endl;
    else if (claw::isString(v)) std::cout << claw::asString(v) << std::endl;
    else std::cout << claw::valueToString(v) << std::endl;
}
extern "C" void claw_vm_get_global(claw::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    if (!vm->apiGlobalExists(namePtr)) {
        std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
        vm->apiPush(claw::nilValue());
        return;
    }
    claw::Value value = vm->apiGlobalGet(namePtr);
    vm->apiPush(value);
}
extern "C" void claw_vm_define_global(claw::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    auto value = vm->apiPop();
    vm->apiDefineGlobal(namePtr, value);
}
extern "C" void claw_vm_set_global(claw::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    if (!vm->apiGlobalExists(namePtr)) {
        std::cerr << "Undefined variable '" << namePtr << "'." << std::endl;
        return;
    }
    vm->apiGlobalAssign(namePtr, vm->apiPeek(0));
}
extern "C" int claw_vm_try_get_global_cached(claw::VM* vm, const char* namePtr, const uint8_t* siteIp, uint64_t* out) {
    claw::Value v;
    int hit = vm->apiTryGetGlobalCached(namePtr, siteIp, &v);
    if (hit && out) *out = v;
    return hit;
}
extern "C" void claw_vm_call(claw::VM* vm) {
    uint8_t argCount = vm->apiReadByte();
    claw::Value callee = vm->apiPeek(argCount);
    vm->apiCallValue(callee, argCount);
}
extern "C" void claw_vm_closure(claw::VM* vm) {
    claw::Value functionVal = vm->apiReadConstant();
    auto function = claw::asVMFunction(functionVal);
    if (!function) {
        std::cerr << "Expected function constant." << std::endl;
        return;
    }
    auto closure = std::make_shared<claw::VMClosure>();
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
    vm->apiPush(claw::vmClosureValue(closure));
}
extern "C" void claw_vm_get_upvalue(claw::VM* vm) {
    uint8_t slot = vm->apiReadByte();
    vm->apiPush(*vm->apiCurrentClosure()->upvalues[slot]->location);
}
extern "C" void claw_vm_set_upvalue(claw::VM* vm) {
    uint8_t slot = vm->apiReadByte();
    *vm->apiCurrentClosure()->upvalues[slot]->location = vm->apiPeek(0);
}
extern "C" void claw_vm_close_upvalue(claw::VM* vm) {
    vm->apiCloseTopUpvalue();
}
extern "C" int claw_vm_osr_enter(claw::VM* vm, const uint8_t* ip) {
    return vm->osrEnter(ip) ? 1 : 0;
}
extern "C" bool claw_vm_return(claw::VM* vm) { return vm->apiReturn(); }
extern "C" void claw_vm_get_property(claw::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    claw::Value instanceVal = vm->apiPeek();
    if (!claw::isInstance(instanceVal)) {
        std::cerr << "Only instances have properties." << std::endl;
        return;
    }
    auto instance = claw::asInstance(instanceVal);
    claw::Token nameToken(claw::TokenType::Identifier, namePtr, 0);
    claw::Value value = instance->get(nameToken);
    vm->apiPop();
    vm->apiPush(value);
}
extern "C" int claw_vm_try_get_property_cached(claw::VM* vm, uint64_t instanceVal, const char* namePtr, const uint8_t* siteIp, uint64_t* out) {
    claw::Value v;
    int hit = vm->apiTryGetPropertyCached(instanceVal, namePtr, siteIp, &v);
    if (hit && out) *out = v;
    return hit;
}
extern "C" int claw_vm_try_call_cached(claw::VM* vm, const uint8_t* siteIp, uint8_t argCount) {
    return vm->apiTryCallCached(siteIp, argCount);
}
extern "C" void claw_vm_set_property(claw::VM* vm) {
    const char* namePtr = vm->apiReadStringPtr();
    claw::Value value = vm->apiPeek(0);
    claw::Value instanceVal = vm->apiPeek(1);
    if (!claw::isInstance(instanceVal)) {
        std::cerr << "Only instances have fields." << std::endl;
        return;
    }
    auto instance = claw::asInstance(instanceVal);
    claw::Token nameToken(claw::TokenType::Identifier, namePtr, 0);
    instance->set(nameToken, value);
    vm->apiPop();
    vm->apiPop();
    vm->apiPush(value);
}
