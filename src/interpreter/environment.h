#pragma once
#include "value.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <string_view>
#include <functional>

namespace claw {

// Variable storage and scoping
class Environment {
public:
    enum class SandboxMode {
        Full,
        Network,
        Strict
    };
    Environment() : enclosing_(nullptr),
                    sandboxMode_(SandboxMode::Full),
                    allowFileRead_(true),
                    allowFileWrite_(true),
                    allowFileDelete_(true),
                    allowInput_(true),
                    allowOutput_(true),
                    allowNetwork_(false),
                    logPath_("claw.log"),
                    logHmacKey_("") {}
    explicit Environment(std::shared_ptr<Environment> enclosing) 
        : enclosing_(enclosing),
          sandboxMode_(SandboxMode::Full),
          allowFileRead_(true),
          allowFileWrite_(true),
          allowFileDelete_(true),
          allowInput_(true),
          allowOutput_(true),
          allowNetwork_(false),
          logPath_("claw.log"),
          logHmacKey_(""),
          logMetaRequired_(false) {}
    
    // Define new variable
    void define(std::string_view name, Value value);
    void define(std::string_view name, std::shared_ptr<class Callable> fn);
    
    // Get variable value
    Value get(std::string_view name) const;
    
    // Assign to existing variable
    void assign(std::string_view name, Value value);
    
    // Check if variable exists
    bool exists(std::string_view name) const;

    // Sandbox
    void setSandbox(SandboxMode mode);
    SandboxMode sandbox() const { return sandboxMode_; }
    bool canFileRead() const { return allowFileRead_; }
    bool canFileWrite() const { return allowFileWrite_; }
    bool canFileDelete() const { return allowFileDelete_; }
    bool canInput() const { return allowInput_; }
    bool canOutput() const { return allowOutput_; }
    bool canNetwork() const { return allowNetwork_; }
    void setFileReadAllowed(bool v) { allowFileRead_ = v; }
    void setFileWriteAllowed(bool v) { allowFileWrite_ = v; }
    void setFileDeleteAllowed(bool v) { allowFileDelete_ = v; }
    void setInputAllowed(bool v) { allowInput_ = v; }
    void setOutputAllowed(bool v) { allowOutput_ = v; }
    void setNetworkAllowed(bool v) { allowNetwork_ = v; }
    void setLogPath(const std::string& p) { logPath_ = p; }
    void setLogHmacKey(const std::string& k) { logHmacKey_ = k; }
    const std::string& logPath() const { return logPath_; }
    const std::string& logHmacKey() const { return logHmacKey_; }
    void setLogMetaRequired(bool v) { logMetaRequired_ = v; }
    bool logMetaRequired() const { return logMetaRequired_; }
    void setDefaultEncryptedIO(bool v) { defaultEncryptedIO_ = v; }
    bool defaultEncryptedIO() const { return defaultEncryptedIO_; }
    void setIoEncPass(const std::string& p) { ioEncPass_ = p; }
    const std::string& ioEncPass() const { return ioEncPass_; }
    void setAntiDebugEnforced(bool v) { antiDebugEnforced_ = v; }
    bool antiDebugEnforced() const { return antiDebugEnforced_; }
    void setDynamicCodeEncryption(bool v) { dynamicCodeEncryption_ = v; }
    bool dynamicCodeEncryption() const { return dynamicCodeEncryption_; }
    void setCryptoPreferred(const std::string& a) { cryptoPreferred_ = a; }
    const std::string& cryptoPreferred() const { return cryptoPreferred_; }

    // Environment Caching (v0.8.6 Optimization)
    struct CacheEntry {
        std::shared_ptr<Environment> env;
        Value value;
        bool found;
    };

    struct ProfileEntry {
        uint64_t hits;
    };

    struct InternedStringHash {
        size_t operator()(std::string_view sv) const {
            return std::hash<const char*>{}(sv.data());
        }
    };

    struct InternedStringEqual {
        bool operator()(std::string_view sv1, std::string_view sv2) const {
            return sv1.data() == sv2.data();
        }
    };
    
    static void clearGlobalCache();
    void forEachValue(const std::function<void(Value)>& fn) const;
    void forEachKey(const std::function<void(std::string_view)>& fn) const;
    std::shared_ptr<Environment> enclosing() const { return enclosing_; }

private:
    // Using string_view as key for performance (guaranteed interned)
    std::unordered_map<std::string_view, Value, InternedStringHash, InternedStringEqual> values_;
    std::shared_ptr<Environment> enclosing_;
    
    // Performance: Fast lookup cache for deeply nested environments
    mutable std::unordered_map<std::string_view, CacheEntry, InternedStringHash, InternedStringEqual> lookup_cache_;
    mutable std::unordered_map<std::string_view, ProfileEntry, InternedStringHash, InternedStringEqual> profile_;

    SandboxMode sandboxMode_;
    bool allowFileRead_;
    bool allowFileWrite_;
    bool allowFileDelete_;
    bool allowInput_;
    bool allowOutput_;
    bool allowNetwork_;
    std::string logPath_;
    std::string logHmacKey_;
    bool logMetaRequired_;
    bool defaultEncryptedIO_ = false;
    std::string ioEncPass_;
    bool antiDebugEnforced_ = false;
    bool dynamicCodeEncryption_ = false;
    std::string cryptoPreferred_ = "AES_GCM";
};

} // namespace claw
