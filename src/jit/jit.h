 #pragma once
 #include <cstdint>
 #include <memory>
 #include <string>
 #include <vector>
 #include "interpreter/value.h"
 
 namespace volt {
 
 enum class JitTier {
     Interpreter,
     Baseline,
     Optimized
 };
 
 struct JitFunction {
     using FnPtr = Value(*)(Value* args, int argc);
     FnPtr fn;
     JitTier tier;
     std::string debugName;
 };
 
 class JitEngine {
 public:
     JitEngine() = default;
     ~JitEngine() = default;
 
     JitFunction compileBaseline(const std::string& name, const std::vector<uint8_t>& bytecode);
     JitFunction compileOptimized(const std::string& name, const std::vector<uint8_t>& bytecode);
 
     void invalidateAll() {}
 };
 
 } // namespace volt

