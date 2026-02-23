#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include "interpreter/value.h"
#ifdef CLAW_ENABLE_JIT
namespace claw { class LlvJitCompiler; }
#endif
namespace claw {
enum class JitTier {
    Interpreter,
    Baseline,
    Optimized
};
struct JitEntry {
    const uint8_t* ip;
    void* fn;
    JitTier tier;
};
struct JitConfig {
    bool aggressive = false;
    uint32_t loopThreshold = 1000;
    uint32_t functionThreshold = 1000;
};
class JitEngine {
public:
    JitEngine();
    ~JitEngine();
    void setConfig(const JitConfig& cfg);
    bool hasBaseline(const void* key) const;
    void* getBaselineEntry(const void* key, const uint8_t* ip) const;
    void registerBaseline(const void* key, const std::vector<JitEntry>& entries);
    bool enterOSR(void* vm, const void* key, const uint8_t* ip);
    void invalidateAll();
private:
    JitConfig config_;
    std::unordered_map<const void*, std::vector<JitEntry>> baseline_;
#ifdef CLAW_ENABLE_JIT
    std::unique_ptr<LlvJitCompiler> compiler_;
#endif
};
extern JitConfig gJitConfig;
} // namespace claw
