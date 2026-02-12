#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <chrono>
#include <iostream>

namespace volt {

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
        "for (let i = 0; i < 100000; i++) {\n"
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
        "    for (let i = 0; i < 100000; i++) {\n"
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
        "for (let i = 0; i < 10000; i++) {\n"
        "  arr.push(i);\n"
        "}\n"
        "let sum = 0;\n"
        "for (let i = 0; i < 10000; i++) {\n"
        "  sum = sum + arr[i];\n"
        "}\n";
    
    long long ms = runAndMeasure(code);
    std::cout << "[ BENCHMARK ] Array 10,000 push & access: " << ms << "ms" << std::endl;
}

} // namespace volt
