#include <benchmark/benchmark.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include <string>
#include <vector>
#include <memory>

static void BM_FibonacciRecursive(benchmark::State& state) {
    std::string source = 
        "fn fib(n) {"
        "  if (n < 2) return n;"
        "  return fib(n-1) + fib(n-2);"
        "}"
        "fib(15);"; // Using 15 to keep it reasonable for repeated runs

    for (auto _ : state) {
        claw::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        claw::Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        claw::Interpreter interpreter;
        interpreter.execute(statements);
    }
}
BENCHMARK(BM_FibonacciRecursive);

static void BM_ArrayOperations(benchmark::State& state) {
    std::string source = 
        "let arr = [];"
        "for (let i = 0; i < 100; i = i + 1) {"
        "  arr.push(i);"
        "}"
        "for (let i = 0; i < 50; i = i + 1) {"
        "  arr.pop();"
        "}"
        "arr.reverse();";

    for (auto _ : state) {
        claw::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        claw::Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        claw::Interpreter interpreter;
        interpreter.execute(statements);
    }
}
BENCHMARK(BM_ArrayOperations);

// Loop benchmark
static void BM_LoopPerformance(benchmark::State& state) {
    std::string source = 
        "let sum = 0;"
        "for (let i = 0; i < 1000; i = i + 1) {"
        "  sum = sum + i;"
        "}";

    for (auto _ : state) {
        claw::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        claw::Parser parser(tokens);
        auto statements = parser.parseProgram();
        
        claw::Interpreter interpreter;
        interpreter.execute(statements);
    }
}
BENCHMARK(BM_LoopPerformance);
