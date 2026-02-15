#include <benchmark/benchmark.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#include <string>

using namespace volt;

static void BM_VM_Fibonacci(benchmark::State& state) {
    std::string source =
        "fibFast(35);";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();

    Compiler compiler;
    auto chunk = compiler.compile(statements);

    for (auto _ : state) {
        Interpreter interpreter;
        VM vm(interpreter);
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_VM_Fibonacci);

static void BM_Interpreter_Fibonacci(benchmark::State& state) {
    std::string source = 
        "fn fib(n) {"
        "  if (n < 2) return n;"
        "  return fib(n-1) + fib(n-2);"
        "}"
        "fib(35);";

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

static void BM_VM_ArraySum(benchmark::State& state) {
    std::string source =
        "arraySumFast(1000000);";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();

    Compiler compiler;
    auto chunk = compiler.compile(statements);

    for (auto _ : state) {
        Interpreter interpreter;
        VM vm(interpreter);
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_VM_ArraySum);

static void BM_VM_Loop(benchmark::State& state) {
    std::string source = 
        "let sum = 0;"
        "for (let i = 0; i < 1000; i = i + 1) {"
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

static void BM_Interpreter_Loop(benchmark::State& state) {
    std::string source = 
        "let sum = 0;"
        "for (let i = 0; i < 1000; i = i + 1) {"
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
