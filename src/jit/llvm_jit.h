#pragma once
#ifdef VOLT_ENABLE_JIT
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
namespace llvm {
namespace orc {
class LLJIT;
}
class LLVMContext;
class Module;
}
namespace volt {
class LlvJitCompiler {
public:
    LlvJitCompiler();
    ~LlvJitCompiler();
    void* compileIp(const uint8_t* ip);
    void* compileFunction(const uint8_t* ip);
private:
    std::unique_ptr<llvm::orc::LLJIT> lljit_;
};
} // namespace volt
#endif
