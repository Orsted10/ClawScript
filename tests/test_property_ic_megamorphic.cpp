#include <gtest/gtest.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#
static std::unique_ptr<volt::Chunk> compileSrc(const std::string& src) {
    volt::Lexer lex(src);
    auto tokens = lex.tokenize();
    volt::Parser parser(tokens);
    auto program = parser.parseProgram();
    volt::Compiler compiler;
    return compiler.compile(program);
}
#
TEST(PropertyICDiagnostics, MegamorphicPromotionAfterMissThreshold) {
    // Define a simple class via interpreter so VM GetProperty uses instance path.
    std::string classDecl =
        "class C { fn init() {} }";
    volt::Lexer lex1(classDecl);
    auto tokens1 = lex1.tokenize();
    volt::Parser parser1(tokens1);
    auto program1 = parser1.parseProgram();
    ASSERT_FALSE(parser1.hadError());
    volt::Interpreter interp;
    interp.execute(program1);
    #
    // VM chunk: create many distinct instances and access o.v at the same site.
    std::string loopSrc =
        "let i = 0;"
        "while (i < 40) {"
        "  let o = C();"
        "  o.v = i;"
        "  print o.v;"
        "  i = i + 1;"
        "}";
    auto chunk = compileSrc(loopSrc);
    volt::VM vm(interp);
    auto res = vm.interpret(*chunk);
    EXPECT_EQ(res, volt::InterpretResult::Ok);
    auto siteIp = vm.apiGetLastPropertySiteIp();
#ifndef VOLT_DISABLE_IC_DIAGNOSTICS
    ASSERT_NE(siteIp, nullptr);
    auto misses = vm.apiGetPropertyMisses(siteIp);
    EXPECT_GE(misses, 17u);
    EXPECT_TRUE(vm.apiIsPropertyMegamorphic(siteIp));
#endif
}
