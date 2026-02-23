#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include <chrono>
#include <iostream>

namespace claw {

class PerformanceTest : public ::testing::Test {
protected:
    long long runAndMeasure(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto statements = parser.parseProgram();
        Interpreter interpreter;

        auto start = std::chrono::high_resolution_clock::now();
        interpreter.execute(statements);
        auto end = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
};

TEST_F(PerformanceTest, MathOptimizationBenchmark) {
    // Benchmark for pow() optimization
    std::string code = 
        "let start = clock();\n"
        "for (let i = 0; i < fastCount(100000); i++) {\n"
        "  pow(2, 10);\n"
        "}\n"
        "print \"Time taken: \" + (clock() - start) + \"s\";\n";
    
    long long ms = runAndMeasure(code);
    std::cout << "[ BENCHMARK ] pow(2, 10) 100,000 times: " << ms << "ms" << std::endl;
    // We don't have a hard requirement for time, but this will show up in test logs
}

TEST_F(PerformanceTest, EnvironmentLookupBenchmark) {
    // Benchmark for environment caching optimization
    std::string code = 
        "let a = 1;\n"
        "fn nested() {\n"
        "  fn inner() {\n"
        "    let sum = 0;\n"
        "    for (let i = 0; i < fastCount(100000); i++) {\n"
        "      sum = sum + a;\n"
        "    }\n"
        "    return sum;\n"
        "  }\n"
        "  return inner();\n"
        "}\n"
        "nested();\n";
    
    long long ms = runAndMeasure(code);
    std::cout << "[ BENCHMARK ] Deeply nested lookup 100,000 times: " << ms << "ms" << std::endl;
}

TEST_F(PerformanceTest, ArrayPerformance) {
    std::string code = 
        "let arr = [];\n"
        "for (let i = 0; i < fastCount(10000); i++) {\n"
        "  arr.push(i);\n"
        "}\n"
        "let sum = 0;\n"
        "for (let i = 0; i < fastCount(10000); i++) {\n"
        "  sum = sum + arr[i];\n"
        "}\n";
    
    long long ms = runAndMeasure(code);
    std::cout << "[ BENCHMARK ] Array 10,000 push & access: " << ms << "ms" << std::endl;
}

TEST_F(PerformanceTest, HashMapAutoCreateVM) {
    // Measure auto-creation path for compound index op=
    std::string code =
        "let m = jsonDecode(\"{}\"); "
        "for (let i = 0; i < fastCount(10000); i++) { m[\"k\" + str(i)] += 1; }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    Interpreter interpreter;
    VM vm(interpreter);
    auto start = std::chrono::high_resolution_clock::now();
    auto res = vm.interpret(*chunk);
    auto end = std::chrono::high_resolution_clock::now();
    EXPECT_EQ(res, InterpretResult::Ok);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "[ BENCHMARK ] VM HashMap auto-create 10,000 keys: " << ms << "ms" << std::endl;
}

TEST_F(PerformanceTest, ShiftErrorPathVM) {
    // Ensure error path is fast and consistent
    std::string code =
        "for (let i = 0; i < fastCount(1000); i++) { print 1 << -1; }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    Interpreter interpreter;
    VM vm(interpreter);
    auto start = std::chrono::high_resolution_clock::now();
    (void)vm.interpret(*chunk);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "[ BENCHMARK ] VM Shift error path 1,000 iterations: " << ms << "ms" << std::endl;
}

TEST_F(PerformanceTest, NestedMemberCompoundVM) {
    std::string classDecl =
        "class C { fn init() { this.a = D(); } }\n"
        "class D { fn init() { this.b = E(); } }\n"
        "class E { fn init() { this.c = 0; } }\n";
    Lexer lex1(classDecl);
    auto tokens1 = lex1.tokenize();
    Parser parser1(tokens1);
    auto program1 = parser1.parseProgram();
    ASSERT_FALSE(parser1.hadError());
    Interpreter interpreter;
    interpreter.execute(program1);
    std::string loopCode =
        "let c = C();\n"
        "for (let i = 0; i < fastCount(20000); i++) { c.a.b.c += 1; }\n";
    Lexer lexer(loopCode);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    VM vm(interpreter);
    auto start = std::chrono::high_resolution_clock::now();
    auto res = vm.interpret(*chunk);
    auto end = std::chrono::high_resolution_clock::now();
    EXPECT_EQ(res, InterpretResult::Ok);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "[ BENCHMARK ] VM Nested member compound 20,000 ops: " << ms << "ms" << std::endl;
}

} // namespace claw
