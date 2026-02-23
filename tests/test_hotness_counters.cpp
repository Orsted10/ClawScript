#include <gtest/gtest.h>
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/value.h"
#include "jit/jit.h"
#include "features/string_pool.h"
#
static std::unique_ptr<claw::Chunk> compileSrc(const std::string& src) {
    claw::Lexer lex(src);
    auto tokens = lex.tokenize();
    claw::Parser parser(tokens);
    auto program = parser.parseProgram();
    claw::Compiler compiler;
    return compiler.compile(program);
}
#
static const claw::VMFunction* getFn(claw::VM& vm, const char* namePtr) {
    claw::Value v = vm.apiGlobalGet(namePtr);
    if (claw::isVMClosure(v)) {
        auto c = claw::asVMClosurePtr(v);
        if (c) return c->function.get();
    }
    if (claw::isVMFunction(v)) {
        auto f = claw::asVMFunction(v);
        if (f) return f.get();
    }
    return nullptr;
}
#
TEST(HotnessCounters, Thresholds_998_999_1000_1001) {
    std::string fn = "fn f(a,b){ return a + b; }";
    auto sv = claw::StringPool::intern(std::string("f"));
    const char* namePtr = sv.data();
    for (int N : {998, 999, 1000, 1001}) {
        std::string body = "let i=0; while(i<" + std::to_string(N) + "){ let r=f(1,2); i=i+1; }";
        auto chunk = compileSrc(fn + body);
        claw::VM vm;
        auto res = vm.interpret(*chunk);
        EXPECT_EQ(res, claw::InterpretResult::Ok);
        auto fnPtr = getFn(vm, namePtr);
        ASSERT_NE(fnPtr, nullptr);
        auto count = vm.apiGetFunctionHotness(fnPtr);
        EXPECT_EQ(count, (uint32_t)N);
    }
}
#
TEST(HotnessCounters, AggressiveJitQuarterThreshold) {
    claw::gJitConfig.aggressive = true;
    claw::gJitConfig.functionThreshold = 1000;
    std::string fn = "fn g(){ return 42; }";
    auto sv = claw::StringPool::intern(std::string("g"));
    const char* namePtr = sv.data();
    int N = 250;
    std::string body = "let i=0; while(i<250){ let r=g(); i=i+1; }";
    auto chunk = compileSrc(fn + body);
    claw::VM vm;
    auto res = vm.interpret(*chunk);
    EXPECT_EQ(res, claw::InterpretResult::Ok);
    auto fnPtr = getFn(vm, namePtr);
    ASSERT_NE(fnPtr, nullptr);
    auto count = vm.apiGetFunctionHotness(fnPtr);
    EXPECT_EQ(count, (uint32_t)N);
#ifdef CLAW_ENABLE_JIT
    bool hasBaseline = vm.apiHasBaseline(fnPtr);
    EXPECT_TRUE(hasBaseline);
#endif
    claw::gJitConfig.aggressive = false;
}
#
TEST(HotnessCounters, InitialZeroAndReset) {
    std::string fn = "fn h(){ return 1; }";
    auto sv = claw::StringPool::intern(std::string("h"));
    const char* namePtr = sv.data();
    auto chunk = compileSrc(fn + "print 1;");
    {
        claw::VM vm;
        auto res = vm.interpret(*chunk);
        EXPECT_EQ(res, claw::InterpretResult::Ok);
        auto fnPtr = getFn(vm, namePtr);
        ASSERT_NE(fnPtr, nullptr);
        EXPECT_GE(vm.apiGetFunctionHotness(fnPtr), (uint32_t)0);
    }
    {
        claw::VM vm2;
        auto res2 = vm2.interpret(*chunk);
        EXPECT_EQ(res2, claw::InterpretResult::Ok);
        auto fnPtr2 = getFn(vm2, namePtr);
        ASSERT_NE(fnPtr2, nullptr);
        EXPECT_GE(vm2.apiGetFunctionHotness(fnPtr2), (uint32_t)0);
    }
}
#
#ifdef CLAW_ENABLE_JIT
#include <thread>
#include <atomic>
TEST(HotnessCounters, CrossThreadJitCompilation) {
    claw::gJitConfig.aggressive = true;
    claw::gJitConfig.functionThreshold = 1000;
    std::string fn = "fn t(){ return 7; }";
    auto sv = claw::StringPool::intern(std::string("t"));
    const char* namePtr = sv.data();
    auto chunk = compileSrc(fn + "let i=0; while(i<250){ let r=t(); i=i+1; }");
    std::atomic<int> ready{0};
    std::atomic<int> done{0};
    auto work = [&]() {
        claw::VM vm;
        ready++;
        while (ready.load() < 2) { std::this_thread::yield(); }
        auto res = vm.interpret(*chunk);
        EXPECT_EQ(res, claw::InterpretResult::Ok);
        auto fnPtr = getFn(vm, namePtr);
        ASSERT_NE(fnPtr, nullptr);
        EXPECT_TRUE(vm.apiHasBaseline(fnPtr));
        done++;
    };
    std::thread th1(work);
    std::thread th2(work);
    th1.join(); th2.join();
    EXPECT_EQ(done.load(), 2);
    claw::gJitConfig.aggressive = false;
}
#endif
