#include <gtest/gtest.h>
#include <chrono>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#
using hrclock = std::chrono::high_resolution_clock;
using ms_t = std::chrono::milliseconds;
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
TEST(BenchmarkAssertions, MandelbrotUnder100ms) {
    std::string source =
        "let w=80; let h=40;"
        "for (let i = 0; i < h; i = i + 1) {"
        "  for (let j = 0; j < w; j = j + 1) {"
        "    let x = (j / 40 - 1.5);"
        "    let y = (i / 20 - 1.0);"
        "    let a = 0; let b = 0; let k = 0;"
        "    while (k < 100) {"
        "      let aa = a * a - b * b + x;"
        "      let bb = 2 * a * b + y;"
        "      a = aa; b = bb;"
        "      if (a * a + b * b > 4) { break; }"
        "      k = k + 1;"
        "    }"
        "  }"
        "}";
    auto chunk = compileSrc(source);
    volt::VM vm;
    auto t0 = hrclock::now();
    auto res = vm.interpret(*chunk);
    auto t1 = hrclock::now();
    EXPECT_EQ(res, volt::InterpretResult::Ok);
    auto elapsed = std::chrono::duration_cast<ms_t>(t1 - t0).count();
    EXPECT_LT(elapsed, 100);
}
#
TEST(BenchmarkAssertions, ObjectMethodLoopUnder10ms) {
    std::string source =
        "let c = {};"
        "c.add = fun(a,b){ return a+b; };"
        "let i = 0; let acc = 0;"
        "while (i < 600) {"
        "  acc = acc + c.add(1,2);"
        "  i = i + 1;"
        "}"
        "print acc;";
    volt::Lexer lex(source);
    auto tokens = lex.tokenize();
    volt::Parser parser(tokens);
    auto program = parser.parseProgram();
    ASSERT_FALSE(parser.hadError());
    auto t0 = hrclock::now();
    volt::Interpreter interp;
    EXPECT_NO_THROW(interp.execute(program));
    auto t1 = hrclock::now();
    auto elapsed = std::chrono::duration_cast<ms_t>(t1 - t0).count();
    EXPECT_LT(elapsed, 10);
}
#
TEST(BenchmarkAssertions, ObjectMethodLoopVMClassUnder10ms) {
    std::string classDecl =
        "class C { fn add(a,b){ return a+b; } }"
        "let c = C();";
    volt::Lexer lex1(classDecl);
    auto tokens1 = lex1.tokenize();
    volt::Parser parser1(tokens1);
    auto program1 = parser1.parseProgram();
    ASSERT_FALSE(parser1.hadError());
    volt::Interpreter interp;
    interp.execute(program1);
    std::string loopSrc =
        "let i = 0; let acc = 0;"
        "while (i < 600) {"
        "  acc = acc + c.add(1,2);"
        "  i = i + 1;"
        "}"
        "print acc;";
    auto chunk = compileSrc(loopSrc);
    volt::VM vm(interp);
    auto t0 = hrclock::now();
    auto res = vm.interpret(*chunk);
    auto t1 = hrclock::now();
    EXPECT_EQ(res, volt::InterpretResult::Ok);
    auto elapsed = std::chrono::duration_cast<ms_t>(t1 - t0).count();
    EXPECT_LT(elapsed, 10);
}
