 #include "llvm_aot.h"
 
 namespace volt {
 
 AotModule AotCompiler::compile(const std::string& name, const std::vector<uint8_t>& bytecode) {
     AotModule m;
     m.name = name;
     m.image = bytecode;
     return m;
 }
 
 } // namespace volt

