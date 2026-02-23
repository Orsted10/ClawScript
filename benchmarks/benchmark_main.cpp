#include <benchmark/benchmark.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#include "features/string_pool.h"
#include <string>
#include <vector>

using namespace claw;

// ========================================
// INTERPRETER BENCHMARKS
// ========================================

static void BM_Interpreter_Loop(benchmark::State& state) {
    std::string source = 
        "let sum = 0;"
        "for (let i = 0; i < 100000; i = i + 1) {"
        "  sum = sum + i;"
        "}";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    for (auto _ : state) {
        Interpreter interpreter;
        interpreter.execute(statements);
    }
}
BENCHMARK(BM_Interpreter_Loop);

static void BM_VM_Loop(benchmark::State& state) {
    std::string source = 
        "let sum = 0;"
        "for (let i = 0; i < 100000; i = i + 1) {"
        "  sum = sum + i;"
        "}";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    Compiler compiler;
    auto chunk = compiler.compile(statements);

    for (auto _ : state) {
        VM vm;
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_VM_Loop);

/*
static void BM_Interpreter_Fibonacci(benchmark::State& state) {
    std::string source = 
        "fn fib(n) {"
        "  if (n < 2) return n;"
        "  return fib(n-1) + fib(n-2);"
        "}"
        "fib(15);";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    for (auto _ : state) {
        Interpreter interpreter;
        interpreter.execute(statements);
    }
}
BENCHMARK(BM_Interpreter_Fibonacci);

static void BM_VM_Fibonacci(benchmark::State& state) {
    std::string source = 
        "fn fib(n) {"
        "  if (n < 2) return n;"
        "  return fib(n-1) + fib(n-2);"
        "}"
        "fib(15);";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    Compiler compiler;
    auto chunk = compiler.compile(statements);

    for (auto _ : state) {
        VM vm;
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_VM_Fibonacci);
*/

// ========================================
// STRING POOL BENCHMARKS
// ========================================

static void BM_StringInterning(benchmark::State& state) {
    std::vector<std::string> strings;
    for (int i = 0; i < 1000; ++i) {
        strings.push_back("string_" + std::to_string(i));
    }

    for (auto _ : state) {
        for (const auto& s : strings) {
            benchmark::DoNotOptimize(StringPool::intern(s));
        }
    }
}
BENCHMARK(BM_StringInterning);

static void BM_StringComparison_Raw(benchmark::State& state) {
    std::string s1 = "a_very_long_string_to_compare_for_performance_reasons_1234567890";
    std::string s2 = "a_very_long_string_to_compare_for_performance_reasons_1234567890";
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(s1 == s2);
    }
}
BENCHMARK(BM_StringComparison_Raw);

static void BM_StringComparison_Interned(benchmark::State& state) {
    std::string_view s1 = StringPool::intern(std::string("a_very_long_string_to_compare_for_performance_reasons_1234567890"));
    std::string_view s2 = StringPool::intern(std::string("a_very_long_string_to_compare_for_performance_reasons_1234567890"));
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(s1.data() == s2.data());
    }
}
BENCHMARK(BM_StringComparison_Interned);

// ========================================
// JIT Benchmarks (Adaptive OSR)
// ========================================
static void BM_MandelbrotJIT(benchmark::State& state) {
    using namespace claw;
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
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    for (auto _ : state) {
        VM vm;
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_MandelbrotJIT);

BENCHMARK_MAIN();
