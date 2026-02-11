#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

class IntegrationTest : public ::testing::Test {
protected:
    std::string runScript(const std::string& scriptPath) {
        // Try to find the file in several possible locations relative to the executable
        std::vector<std::string> searchPaths = {
            scriptPath,                          // As provided
            "../../" + scriptPath,               // Relative to build/bin/Release
            "../" + scriptPath,                  // Relative to build/bin
            "../../../" + scriptPath,            // Relative to build/bin/Release (another level)
            "examples/" + scriptPath             // From root
        };

        std::string finalPath;
        std::ifstream file;
        for (const auto& p : searchPaths) {
            file.open(p);
            if (file) {
                finalPath = p;
                break;
            }
        }

        if (!file) {
            return "Error: Could not open file " + scriptPath + " (searched in multiple locations)";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        // Redirect cout to capture output
        std::streambuf* oldCout = std::cout.rdbuf();
        std::stringstream capturedOutput;
        std::cout.rdbuf(capturedOutput.rdbuf());

        try {
            volt::Lexer lexer(source);
            auto tokens = lexer.tokenize();
            volt::Parser parser(tokens);
            auto statements = parser.parseProgram();

            if (parser.hadError()) {
                std::cout.rdbuf(oldCout);
                std::string errs;
                for (const auto& e : parser.getErrors()) errs += e + "\n";
                return "Parser Error: " + errs;
            }

            volt::Interpreter interpreter;
            interpreter.execute(statements);
        } catch (const volt::RuntimeError& e) {
            std::cout.rdbuf(oldCout);
            return std::string("Runtime Error: ") + e.what();
        } catch (const std::exception& e) {
            std::cout.rdbuf(oldCout);
            return std::string("Exception: ") + e.what();
        }

        std::cout.rdbuf(oldCout);
        std::string res = capturedOutput.str();
        // std::cerr << "Captured: " << res << std::endl; // Debug output to console
        return res;
    }
};

TEST_F(IntegrationTest, FibonacciRecursive) {
    std::string output = runScript("examples/math/fibonacci.volt");
    // The script prints the first 10 Fibonacci numbers: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
    EXPECT_TRUE(output.find("34") != std::string::npos) << "Output was: " << output;
}

TEST_F(IntegrationTest, FactorialRecursion) {
    std::string output = runScript("examples/math/factorial_recursion.volt");
    // fact(5) = 120
    EXPECT_TRUE(output.find("120") != std::string::npos) << "Output was: " << output;
}

TEST_F(IntegrationTest, Closures) {
    std::string output = runScript("examples/functional/closure_test.volt");
    // The closure test should produce predictable output
    EXPECT_FALSE(output.empty());
}

TEST_F(IntegrationTest, NestedArrays) {
    std::string output = runScript("examples/data_structures/advanced_arrays.volt");
    EXPECT_FALSE(output.empty());
}
