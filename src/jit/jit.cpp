#include "jit.h"
#include "vm/vm.h"
#include <algorithm>
#include <cstring>
#ifdef VOLT_ENABLE_JIT
#include "llvm_jit.h"
#endif
namespace volt {
JitEngine::JitEngine() : config_()
#ifdef VOLT_ENABLE_JIT
    , compiler_(std::make_unique<LlvJitCompiler>())
#endif
{}
JitEngine::~JitEngine() = default;
void JitEngine::setConfig(const JitConfig& cfg) { config_ = cfg; }
bool JitEngine::hasBaseline(const void* key) const {
    return baseline_.find(key) != baseline_.end();
}
void* JitEngine::getBaselineEntry(const void* key, const uint8_t* ip) const {
    auto it = baseline_.find(key);
    if (it == baseline_.end()) return nullptr;
    const auto& vec = it->second;
    for (const auto& e : vec) {
        if (e.ip == ip) return e.fn;
    }
    return nullptr;
}
void JitEngine::registerBaseline(const void* key, const std::vector<JitEntry>& entries) {
    std::vector<JitEntry> out;
    out.reserve(entries.size());
    for (auto e : entries) {
#ifdef VOLT_ENABLE_JIT
        if (!e.fn && compiler_) {
            const VMFunction* fnKey = reinterpret_cast<const VMFunction*>(key);
            if (fnKey && fnKey->chunk && e.ip == fnKey->chunk->code().data()) {
                e.fn = compiler_->compileFunction(e.ip);
            } else {
                e.fn = compiler_->compileIp(e.ip);
            }
        }
#endif
        out.push_back(e);
    }
    baseline_[key] = std::move(out);
}
bool JitEngine::enterOSR(void* vm, const void* key, const uint8_t* ip) {
    auto fn = getBaselineEntry(key, ip);
    auto vmp = reinterpret_cast<VM*>(vm);
#ifdef VOLT_ENABLE_JIT
    if (fn) {
        using EntryFn = bool(*)(void*);
        return reinterpret_cast<EntryFn>(fn)(vm);
    }
#endif
    if (!fn) return vmp->osrEnter(ip);
    return false;
}
void JitEngine::invalidateAll() {
    baseline_.clear();
}
JitConfig gJitConfig;
} // namespace volt
