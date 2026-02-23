#include "llvm_jit.h"
#ifdef CLAW_ENABLE_JIT
#include "vm/vm.h"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/AbsoluteSymbols.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
extern "C" int claw_vm_osr_enter(claw::VM* vm, const uint8_t* ip);
extern "C" void claw_vm_set_ip(claw::VM* vm, const uint8_t* ip);
extern "C" const uint8_t* claw_vm_get_ip(claw::VM* vm);
extern "C" uint8_t claw_vm_read_byte(claw::VM* vm);
extern "C" uint64_t claw_vm_read_constant(claw::VM* vm);
extern "C" const char* claw_vm_read_string_ptr(claw::VM* vm);
extern "C" void claw_vm_push(claw::VM* vm, uint64_t v);
extern "C" uint64_t claw_vm_pop(claw::VM* vm);
extern "C" uint64_t claw_vm_peek(claw::VM* vm, int distance);
extern "C" void claw_vm_set_local(claw::VM* vm, int slot, uint64_t v);
extern "C" uint64_t claw_vm_get_local(claw::VM* vm, int slot);
extern "C" void claw_vm_jump(claw::VM* vm, uint16_t offset);
extern "C" void claw_vm_jump_if_false(claw::VM* vm, uint16_t offset);
extern "C" void claw_vm_loop(claw::VM* vm, uint16_t offset);
extern "C" void claw_vm_print(claw::VM* vm);
extern "C" void claw_vm_define_global(claw::VM* vm);
extern "C" void claw_vm_set_global(claw::VM* vm);
extern "C" int claw_vm_try_get_global_cached(claw::VM* vm, const char* namePtr, const uint8_t* siteIp, uint64_t* out);
extern "C" void claw_vm_call(claw::VM* vm);
extern "C" void claw_vm_closure(claw::VM* vm);
extern "C" void claw_vm_get_upvalue(claw::VM* vm);
extern "C" void claw_vm_set_upvalue(claw::VM* vm);
extern "C" void claw_vm_close_upvalue(claw::VM* vm);
extern "C" bool claw_vm_return(claw::VM* vm);
extern "C" void claw_vm_set_property(claw::VM* vm);
extern "C" void claw_vm_get_property(claw::VM* vm);
extern "C" int claw_vm_try_get_property_cached(claw::VM* vm, uint64_t instanceVal, const char* namePtr, const uint8_t* siteIp, uint64_t* out);
extern "C" int claw_vm_try_call_cached(claw::VM* vm, const uint8_t* siteIp, uint8_t argCount);
namespace claw {
LlvJitCompiler::LlvJitCompiler() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    auto jitExp = llvm::orc::LLJITBuilder().create();
    if (!jitExp) {
        lljit_ = nullptr;
        return;
    }
    lljit_ = std::move(*jitExp);
    auto &ES = lljit_->getExecutionSession();
    auto &JD = lljit_->getMainJITDylib();
    llvm::orc::SymbolMap SM;
    SM[ES.intern("claw_vm_osr_enter")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_osr_enter, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_set_ip")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_set_ip, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_get_ip")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_get_ip, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_read_byte")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_read_byte, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_read_constant")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_read_constant, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_read_string_ptr")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_read_string_ptr, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_push")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_push, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_pop")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_pop, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_peek")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_peek, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_set_local")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_set_local, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_get_local")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_get_local, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_jump")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_jump, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_jump_if_false")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_jump_if_false, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_loop")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_loop, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_print")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_print, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_define_global")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_define_global, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_set_global")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_set_global, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_try_get_global_cached")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_try_get_global_cached, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_call")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_call, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_closure")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_closure, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_get_upvalue")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_get_upvalue, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_set_upvalue")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_set_upvalue, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_close_upvalue")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_close_upvalue, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_return")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_return, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_set_property")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_set_property, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_get_property")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_get_property, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_try_get_property_cached")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_try_get_property_cached, llvm::JITSymbolFlags::Exported);
    SM[ES.intern("claw_vm_try_call_cached")] = llvm::orc::ExecutorSymbolDef::fromPtr(&claw_vm_try_call_cached, llvm::JITSymbolFlags::Exported);
    JD.define(llvm::orc::absoluteSymbols(std::move(SM)));
}
LlvJitCompiler::~LlvJitCompiler() = default;
void* LlvJitCompiler::compileIp(const uint8_t* ip) {
    if (!lljit_) return nullptr;
    auto tsc = std::make_unique<llvm::LLVMContext>();
    auto context = tsc.get();
    auto module = std::make_unique<llvm::Module>("claw_jit_ip", *context);
    auto i8 = llvm::Type::getInt8Ty(*context);
    auto i8Ptr = llvm::PointerType::get(*context, 0);
    auto voidPtr = i8Ptr;
    auto boolTy = llvm::Type::getInt1Ty(*context);
    auto fnTy = llvm::FunctionType::get(boolTy, {voidPtr}, false);
    auto fn = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage, "claw_entry", module.get());
    auto entry = llvm::BasicBlock::Create(*context, "entry", fn);
    llvm::IRBuilder<> builder(*context);
    builder.SetInsertPoint(entry);
    auto osrTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), {voidPtr, i8Ptr}, false);
    auto osrDecl = module->getOrInsertFunction("claw_vm_osr_enter", osrTy);
    auto vmArg = &*fn->arg_begin();
    auto ipConst = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), reinterpret_cast<uint64_t>(ip));
    auto ipCast = builder.CreateIntToPtr(ipConst, i8Ptr);
    auto call = builder.CreateCall(osrDecl, {vmArg, ipCast});
    auto cmp = builder.CreateICmpNE(call, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    builder.CreateRet(cmp);
    llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(tsc));
    if (auto err = lljit_->addIRModule(std::move(tsm))) {
        return nullptr;
    }
    auto addrExp = lljit_->lookup("claw_entry");
    if (!addrExp) return nullptr;
    return addrExp->toPtr<void*>();
}
void* LlvJitCompiler::compileFunction(const uint8_t* ipStart) {
    if (!lljit_) return nullptr;
    auto tsc = std::make_unique<llvm::LLVMContext>();
    auto context = tsc.get();
    auto module = std::make_unique<llvm::Module>("claw_jit_fn", *context);
    auto i8Ptr = llvm::PointerType::get(*context, 0);
    auto voidPtr = i8Ptr;
    auto i1 = llvm::Type::getInt1Ty(*context);
    auto fnTy = llvm::FunctionType::get(i1, {voidPtr}, false);
    auto fn = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage, "claw_fn_entry", module.get());
    auto entry = llvm::BasicBlock::Create(*context, "entry", fn);
    llvm::IRBuilder<> builder(*context);
    builder.SetInsertPoint(entry);
    auto vmArg = &*fn->arg_begin();
    auto ipConst = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), reinterpret_cast<uint64_t>(ipStart));
    auto ipCast = builder.CreateIntToPtr(ipConst, i8Ptr);
    auto osrTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), {voidPtr, i8Ptr}, false);
    auto osrDecl = module->getOrInsertFunction("claw_vm_osr_enter", osrTy);
    auto call = builder.CreateCall(osrDecl, {vmArg, ipCast});
    auto cmp = builder.CreateICmpNE(call, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    builder.CreateRet(cmp);
    llvm::orc::ThreadSafeModule tsm(std::move(module), std::move(tsc));
    if (auto err = lljit_->addIRModule(std::move(tsm))) return nullptr;
    auto addrExp = lljit_->lookup("claw_fn_entry");
    if (!addrExp) return nullptr;
    return addrExp->toPtr<void*>();
}
} // namespace claw
#endif
