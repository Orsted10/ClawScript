#include <benchmark/benchmark.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "jit/jit.h"
static void BM_MandelbrotJIT(benchmark::State& state) {
    const char* src =
        "let w=80;let h=40;let i=0;while(i<h){let j=0;while(j<w){let x=(j/40-1.5);let y=(i/20-1.0);"
        "let a=0;let b=0;let k=0;while(k<100){let aa=a*a-b*b+x;let bb=2*a*b+y;a=aa;b=bb;if(a*a+b*b>4){break;}k=k+1;}j=j+1;}i=i+1;}";
    claw::gJitConfig.aggressive = true;
    claw::gJitConfig.loopThreshold = 1000;
    claw::gJitConfig.functionThreshold = 1000;
    for (auto _ : state) {
        claw::Lexer lexer(src);
        auto tokens = lexer.tokenize();
        claw::Parser parser(tokens);
        auto statements = parser.parseProgram();
        claw::Compiler compiler;
        auto chunk = compiler.compile(statements);
        claw::VM vm;
        vm.interpret(*chunk);
    }
}
BENCHMARK(BM_MandelbrotJIT)->Unit(benchmark::kMillisecond);
