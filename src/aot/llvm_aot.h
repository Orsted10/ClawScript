 #pragma once
 #include <memory>
 #include <string>
 #include <vector>
 #include <cstdint>
 #include "interpreter/value.h"
 
 namespace volt {
 
 struct AotModule {
     std::string name;
     std::vector<uint8_t> image;
 };
 
 class AotCompiler {
 public:
     AotCompiler() = default;
     ~AotCompiler() = default;
 
     AotModule compile(const std::string& name, const std::vector<uint8_t>& bytecode);
 };
 
 } // namespace volt

