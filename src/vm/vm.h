#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <array>
#include "chunk.h"
#include "interpreter/value.h"
#include "interpreter/environment.h"
#ifdef CLAW_ENABLE_JIT
#include "jit/jit.h"
#endif

namespace claw {

class Interpreter;

struct RuntimeFlags {
    bool disableCallIC = false;
    bool icDiagnostics = false;
    bool idsEnabled = false;
    int idsStackMax = 64;
    uint64_t idsAllocRateMax = 0;
};
extern RuntimeFlags gRuntimeFlags;

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
    ~VM();

    static constexpr int STACK_MAX = 256;
    static constexpr int FRAMES_MAX = 64;

    InterpretResult interpret(const Chunk& chunk);
    bool osrEnter(const uint8_t* ip);

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
    struct PropertyInlineCacheEntry {
        const ClawInstance* instance;
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
#ifdef CLAW_ENABLE_JIT
    JitEngine jit_;
#endif
    
    std::shared_ptr<Environment> globals_;
    Interpreter* interpreter_;
    uint64_t globalVersion_;
    std::unordered_map<const uint8_t*, GlobalInlineCache> globalInlineCache_;
    std::unordered_map<const uint8_t*, std::vector<PropertyInlineCacheEntry>> propertyInlineCache_;
    std::unordered_map<const ClawInstance*, uint64_t> instanceVersions_;
    std::unordered_map<const uint8_t*, CallInlineCache> callInlineCache_;
    std::unordered_map<const VMFunction*, std::atomic<uint32_t>> functionHotness_;
    std::unordered_map<const uint8_t*, std::atomic<uint32_t>> loopHotness_;
#ifndef CLAW_DISABLE_IC_DIAGNOSTICS
    std::unordered_map<const uint8_t*, uint32_t> propertyICMissCount_;
    std::unordered_set<const uint8_t*> propertyICMegamorphic_;
#endif
    const uint8_t* lastPropertySiteIp_ = nullptr;
#ifdef CLAW_ENABLE_JIT
    JitConfig jitConfig_;
#endif
    std::unique_ptr<Interpreter> ownedInterpreter_;
public:
    void forEachRoot(const std::function<void(Value)>& fn) const;
    uint8_t apiReadByte();
    uint16_t apiReadShort();
    uint64_t apiReadConstant();
    const char* apiReadStringPtr();
    void apiSetIp(const uint8_t* ip);
    const uint8_t* apiGetIp() const;
    void apiPush(Value v);
    Value apiPop();
    Value apiPeek(int distance = 0) const;
    void apiSetLocal(int slot, Value v);
    Value apiGetLocal(int slot);
    void apiJump(uint16_t offset);
    void apiJumpIfFalse(uint16_t offset);
    void apiLoop(uint16_t offset);
    bool apiReturn();
    bool apiIsFalsey(Value v) const;
    void apiDefineGlobal(const char* name, Value v);
    bool apiGlobalExists(const char* name) const;
    Value apiGlobalGet(const char* name) const;
    void apiGlobalAssign(const char* name, Value v);
    void apiBumpGlobalVersion();
    std::shared_ptr<VMUpvalue> apiCaptureUpvalue(Value* local);
    Value* apiCurrentSlots();
    bool apiCallValue(Value callee, int argCount);
    VMClosure* apiCurrentClosure();
    void apiCloseTopUpvalue();
    int apiTryGetGlobalCached(const char* name, const uint8_t* siteIp, Value* out);
    int apiTryGetPropertyCached(Value instanceVal, const char* name, const uint8_t* siteIp, Value* out);
    int apiTryCallCached(const uint8_t* siteIp, uint8_t argCount);
    uint32_t apiGetFunctionHotness(const VMFunction* fn);
    uint32_t apiGetLoopHotness(const uint8_t* ip);
#ifdef CLAW_ENABLE_JIT
    bool apiHasBaseline(const VMFunction* fn);
#endif
    const uint8_t* apiGetLastPropertySiteIp() const { return lastPropertySiteIp_; }
#ifndef CLAW_DISABLE_IC_DIAGNOSTICS
    uint32_t apiGetPropertyMisses(const uint8_t* siteIp) const {
        auto it = propertyICMissCount_.find(siteIp);
        return it == propertyICMissCount_.end() ? 0u : it->second;
    }
    bool apiIsPropertyMegamorphic(const uint8_t* siteIp) const {
        return propertyICMegamorphic_.count(siteIp) > 0;
    }
#endif
};

} // namespace claw
