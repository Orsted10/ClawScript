#include <benchmark/benchmark.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
using namespace claw;
static void BM_VM_PolicyOnOff(benchmark::State& state) {
    std::string source =
        "let s = 0;"
        "for (let i = 0; i < 10000; i = i + 1) {"
        "  s = s + i;"
        "}"
        "print(num(s));";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    Compiler compiler;
    auto chunk = compiler.compile(statements);
    for (auto _ : state) {
        Interpreter interpreter;
        VM vm(interpreter);
        gRuntimeFlags.idsEnabled = false;
        vm.interpret(*chunk);
        gRuntimeFlags.idsEnabled = true;
        gRuntimeFlags.idsStackMax = 64;
        gRuntimeFlags.idsAllocRateMax = 1000000;
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_VM_PolicyOnOff)->Unit(benchmark::kMillisecond);
