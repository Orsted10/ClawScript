 #include "jit.h"
 
 namespace volt {
 
 static Value stubTrampoline(Value*, int) {
     return nilValue();
 }
 
 JitFunction JitEngine::compileBaseline(const std::string& name, const std::vector<uint8_t>&) {
     JitFunction fn;
     fn.fn = &stubTrampoline;
     fn.tier = JitTier::Baseline;
     fn.debugName = name;
     return fn;
 }
 
 JitFunction JitEngine::compileOptimized(const std::string& name, const std::vector<uint8_t>&) {
     JitFunction fn;
     fn.fn = &stubTrampoline;
     fn.tier = JitTier::Optimized;
     fn.debugName = name;
     return fn;
 }
 
 } // namespace volt

