#include "llvm_aot.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include "interpreter/interpreter.h"
#include "vm/vm.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar/LoopVectorize.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/LegacyPassManager.h>
#include <cstring>
#include <stdexcept>

namespace volt {

static uint64_t toBits(double value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(double));
    return bits;
}

static double fromBits(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(double));
    return value;
}

extern "C" int volt_aot_run(const uint8_t* code,
                            uint64_t code_size,
                            const AotConstant* constants,
                            uint64_t constant_count) {
    Chunk chunk;
    for (uint64_t i = 0; i < code_size; i++) {
        chunk.write(code[i], 0);
    }

    for (uint64_t i = 0; i < constant_count; i++) {
        const auto& constant = constants[i];
        Value value = nilValue();
        switch (static_cast<AotConstTag>(constant.tag)) {
            case AotConstTag::Nil:
                value = nilValue();
                break;
            case AotConstTag::Bool:
                value = boolValue(constant.payload != 0);
                break;
            case AotConstTag::Number:
                value = numberToValue(fromBits(constant.payload));
                break;
            case AotConstTag::String: {
                const char* text = reinterpret_cast<const char*>(constant.payload);
                auto sv = StringPool::intern(text ? text : "");
                value = stringValue(sv.data());
                break;
            }
            default:
                value = nilValue();
                break;
        }
        chunk.addConstant(value);
    }

    Interpreter interpreter;
    VM vm(interpreter);
    auto result = vm.interpret(chunk);
    return result == InterpretResult::Ok ? 0 : 1;
}

AotModule AotCompiler::compile(const std::string& name, const Chunk& chunk) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::LLVMContext context;
    auto module = std::make_unique<llvm::Module>(name, context);
    std::string triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);

    auto i8 = llvm::Type::getInt8Ty(context);
    auto i32 = llvm::Type::getInt32Ty(context);
    auto i64 = llvm::Type::getInt64Ty(context);
    auto i8Ptr = llvm::Type::getInt8PtrTy(context);

    auto paddingTy = llvm::ArrayType::get(i8, 7);
    auto constStructTy = llvm::StructType::create(context, "AotConstant");
    constStructTy->setBody({i8, paddingTy, i64});

    auto codeArray = llvm::ConstantDataArray::get(context, chunk.code());
    auto codeGlobal = new llvm::GlobalVariable(
        *module,
        codeArray->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        codeArray,
        "volt_code");
    codeGlobal->setAlignment(llvm::Align(1));

    auto codePtr = llvm::ConstantExpr::getPointerCast(codeGlobal, i8Ptr);
    auto codeSize = llvm::ConstantInt::get(i64, chunk.code().size());

    std::vector<llvm::Constant*> constEntries;
    constEntries.reserve(chunk.constants().size());

    for (const auto& constant : chunk.constants()) {
        uint8_t tag = static_cast<uint8_t>(AotConstTag::Nil);
        llvm::Constant* payloadConst = llvm::ConstantInt::get(i64, 0);

        if (isNil(constant)) {
            tag = static_cast<uint8_t>(AotConstTag::Nil);
        } else if (isBool(constant)) {
            tag = static_cast<uint8_t>(AotConstTag::Bool);
            payloadConst = llvm::ConstantInt::get(i64, asBool(constant) ? 1 : 0);
        } else if (isNumber(constant)) {
            tag = static_cast<uint8_t>(AotConstTag::Number);
            payloadConst = llvm::ConstantInt::get(i64, toBits(asNumber(constant)));
        } else if (isString(constant)) {
            tag = static_cast<uint8_t>(AotConstTag::String);
            const char* text = asStringPtr(constant);
            auto strConst = llvm::ConstantDataArray::getString(context, text ? text : "", true);
            auto strGlobal = new llvm::GlobalVariable(
                *module,
                strConst->getType(),
                true,
                llvm::GlobalValue::PrivateLinkage,
                strConst,
                "volt_str");
            strGlobal->setAlignment(llvm::Align(1));
            auto strPtr = llvm::ConstantExpr::getPointerCast(strGlobal, i8Ptr);
            payloadConst = llvm::ConstantExpr::getPtrToInt(strPtr, i64);
        } else {
            throw std::runtime_error("Unsupported constant type for AOT");
        }

        auto tagConst = llvm::ConstantInt::get(i8, tag);
        auto padConst = llvm::ConstantAggregateZero::get(paddingTy);
        auto entry = llvm::ConstantStruct::get(constStructTy, {tagConst, padConst, payloadConst});
        constEntries.push_back(entry);
    }

    llvm::Constant* constArray;
    llvm::GlobalVariable* constGlobal = nullptr;
    llvm::Constant* constPtr = llvm::ConstantPointerNull::get(constStructTy->getPointerTo());
    llvm::Constant* constCount = llvm::ConstantInt::get(i64, 0);

    if (!constEntries.empty()) {
        auto constArrayTy = llvm::ArrayType::get(constStructTy, constEntries.size());
        constArray = llvm::ConstantArray::get(constArrayTy, constEntries);
        constGlobal = new llvm::GlobalVariable(
            *module,
            constArrayTy,
            true,
            llvm::GlobalValue::PrivateLinkage,
            constArray,
            "volt_consts");
        constGlobal->setAlignment(llvm::Align(8));
        constPtr = llvm::ConstantExpr::getPointerCast(constGlobal, constStructTy->getPointerTo());
        constCount = llvm::ConstantInt::get(i64, constEntries.size());
    }

    auto runTy = llvm::FunctionType::get(i32, {i8Ptr, i64, constStructTy->getPointerTo(), i64}, false);
    auto runFn = module->getOrInsertFunction("volt_aot_run", runTy);

    auto mainTy = llvm::FunctionType::get(i32, false);
    auto mainFn = llvm::Function::Create(mainTy, llvm::Function::ExternalLinkage, "main", module.get());
    auto entry = llvm::BasicBlock::Create(context, "entry", mainFn);
    llvm::IRBuilder<> builder(entry);
    auto call = builder.CreateCall(runFn, {codePtr, codeSize, constPtr, constCount});
    builder.CreateRet(call);

    llvm::PassBuilder passBuilder;
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    passBuilder.registerModuleAnalyses(mam);
    passBuilder.registerFunctionAnalyses(fam);
    passBuilder.registerCGSCCAnalyses(cgam);
    passBuilder.registerLoopAnalyses(lam);
    passBuilder.crossRegisterProxies(lam, fam, cgam, mam);

    auto modulePasses = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    modulePasses.run(*module, mam);

    llvm::FunctionPassManager fpm;
    fpm.addPass(llvm::LoopVectorizePass());
    llvm::ModulePassManager vectorizePass;
    vectorizePass.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
    vectorizePass.run(*module, mam);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        throw std::runtime_error(error);
    }

    llvm::TargetOptions options;
    auto relocModel = std::optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(
        triple,
        llvm::sys::getHostCPUName().str(),
        "",
        options,
        relocModel);
    module->setDataLayout(targetMachine->createDataLayout());

    llvm::SmallVector<char, 0> buffer;
    llvm::raw_svector_ostream dest(buffer);
    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        throw std::runtime_error("TargetMachine can't emit object file");
    }
    pass.run(*module);
    dest.flush();

    AotModule result;
    result.name = name;
    result.image.assign(buffer.begin(), buffer.end());
    return result;
}

} // namespace volt
