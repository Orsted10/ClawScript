 #pragma once
 #include <memory>
 #include <string>
 #include <vector>
 #include <cstdint>
#include "interpreter/value.h"
#include "vm/chunk.h"
 
 namespace claw {
 
 struct AotModule {
     std::string name;
     std::vector<uint8_t> image;
 };
 
enum class AotConstTag : uint8_t {
    Nil = 0,
    Bool = 1,
    Number = 2,
    String = 3
};

struct AotConstant {
    uint8_t tag;
    uint8_t padding[7];
    uint64_t payload;
};

 class AotCompiler {
 public:
     AotCompiler() = default;
     ~AotCompiler() = default;
 
    AotModule compile(const std::string& name, const Chunk& chunk);
 };
 
 } // namespace claw
