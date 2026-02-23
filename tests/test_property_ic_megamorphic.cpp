#include <gtest/gtest.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
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
TEST(PropertyICDiagnostics, MegamorphicPromotionAfterMissThreshold) {
    // Define a simple class via interpreter so VM GetProperty uses instance path.
    std::string classDecl =
        "class C { fn init() {} }";
    claw::Lexer lex1(classDecl);
    auto tokens1 = lex1.tokenize();
    claw::Parser parser1(tokens1);
    auto program1 = parser1.parseProgram();
    ASSERT_FALSE(parser1.hadError());
    claw::Interpreter interp;
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
    claw::VM vm(interp);
    auto res = vm.interpret(*chunk);
    EXPECT_EQ(res, claw::InterpretResult::Ok);
    auto siteIp = vm.apiGetLastPropertySiteIp();
#ifndef CLAW_DISABLE_IC_DIAGNOSTICS
    ASSERT_NE(siteIp, nullptr);
    auto misses = vm.apiGetPropertyMisses(siteIp);
    EXPECT_GE(misses, 17u);
    EXPECT_TRUE(vm.apiIsPropertyMegamorphic(siteIp));
#endif
}

TEST(VMCompoundLowering, MemberPlusEqualStringNumber) {
    std::string classDecl = "class D { fn init() { this.s = \"a\"; } }";
    claw::Lexer lex(classDecl);
    auto toks = lex.tokenize();
    claw::Parser parser(toks);
    auto prog = parser.parseProgram();
    ASSERT_FALSE(parser.hadError());
    claw::Interpreter interp;
    interp.execute(prog);
    std::string src = "let o = D(); o.s += 7; print o.s;";
    auto chunk = compileSrc(src);
    claw::VM vm(interp);
    std::stringstream ss;
    auto old = std::cout.rdbuf(ss.rdbuf());
    auto res = vm.interpret(*chunk);
    std::cout.rdbuf(old);
    EXPECT_EQ(res, claw::InterpretResult::Ok);
    EXPECT_EQ(ss.str(), "a7\n");
}

TEST(VMCompoundLowering, MemberBitwiseAndShifts) {
    std::string classDecl = "class E { fn init() { this.n = 5; } }";
    claw::Lexer lex(classDecl);
    auto toks = lex.tokenize();
    claw::Parser parser(toks);
    auto prog = parser.parseProgram();
    ASSERT_FALSE(parser.hadError());
    claw::Interpreter interp;
    interp.execute(prog);
    std::string src = "let o = E(); o.n <<= 2; print o.n; o.n ^= 3; print o.n; o.n &= 6; print o.n;";
    auto chunk = compileSrc(src);
    claw::VM vm(interp);
    std::stringstream ss;
    auto old = std::cout.rdbuf(ss.rdbuf());
    auto res = vm.interpret(*chunk);
    std::cout.rdbuf(old);
    EXPECT_EQ(res, claw::InterpretResult::Ok);
    EXPECT_EQ(ss.str(), "20\n23\n6\n");
}

TEST(VMCompoundLowering, NestedMemberChains) {
    std::string classDecl =
        "class C { fn init() { this.a = nil; } }\n"
        "class D { fn init() { this.b = nil; } }\n"
        "class E { fn init() { this.c = 1; } }";
    claw::Lexer lex(classDecl);
    auto toks = lex.tokenize();
    claw::Parser parser(toks);
    auto prog = parser.parseProgram();
    ASSERT_FALSE(parser.hadError());
    claw::Interpreter interp;
    interp.execute(prog);
    // Initialize nested chain and perform compound op
    std::string src =
        "let e = E(); let d = D(); d.b = e; let c = C(); c.a = d; c.a.b.c += 4; print c.a.b.c;";
    auto chunk = compileSrc(src);
    claw::VM vm(interp);
    std::stringstream ss;
    auto old = std::cout.rdbuf(ss.rdbuf());
    auto res = vm.interpret(*chunk);
    std::cout.rdbuf(old);
    EXPECT_EQ(res, claw::InterpretResult::Ok);
    EXPECT_EQ(ss.str(), "5\n");
}

TEST(VMCompoundLowering, NestedMemberMissingIntermediate) {
    std::string classDecl =
        "class C { fn init() { } }\n"
        "class D { fn init() { this.b = nil; } }\n"
        "class E { fn init() { this.c = 1; } }";
    claw::Lexer lex(classDecl);
    auto toks = lex.tokenize();
    claw::Parser parser(toks);
    auto prog = parser.parseProgram();
    ASSERT_FALSE(parser.hadError());
    claw::Interpreter interp;
    interp.execute(prog);
    std::string src = "let c = C(); c.a.b.c += 1;";
    auto chunk = compileSrc(src);
    claw::VM vm(interp);
    auto res = vm.interpret(*chunk);
    EXPECT_EQ(res, claw::InterpretResult::RuntimeError);
}
