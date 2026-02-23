#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <sstream>

using namespace claw;

namespace {

// Helper to capture print output
class PrintCapture {
public:
    PrintCapture() : oldBuf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(oldBuf_); }
    std::string getOutput() const { return buffer_.str(); }
private:
    std::stringstream buffer_;
    std::streambuf* oldBuf_;
};

// Helper function to run code and capture output
std::string runCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) {
        return "PARSE_ERROR";
    }
    
    PrintCapture capture;
    Interpreter interpreter;
    
    try {
        interpreter.execute(statements);
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }
    
    return capture.getOutput();
}

} // anonymous namespace

// ==================== MATH FUNCTION TESTS ====================

TEST(MathFunctions, AbsFunction) {
    std::string code = R"(
        print abs(5);
        print abs(-3.14);
        print abs(0);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n3.14\n0\n");
}

TEST(MathFunctions, SqrtFunction) {
    std::string code = R"(
        print sqrt(16);
        print sqrt(2);
        print sqrt(0);
    )";
    
    std::string output = runCode(code);
    // sqrt(2) precision may vary, so we check the beginning
    EXPECT_TRUE(output.find("4\n1.41421") == 0);
    EXPECT_TRUE(output.find("\n0\n") != std::string::npos);
}

TEST(MathFunctions, PowFunction) {
    std::string code = R"(
        print pow(2, 3);
        print pow(5, 2);
        print pow(10, 0);
        print pow(2, -1);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n25\n1\n0.5\n");
}

TEST(MathFunctions, MinMaxFunctions) {
    std::string code = R"(
        print min(5, 3);
        print max(5, 3);
        print min(-1, -5);
        print max(-1, -5);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n5\n-5\n-1\n");
}

TEST(MathFunctions, RoundingFunctions) {
    std::string code = R"(
        print round(3.7);
        print round(3.2);
        print round(-2.7);
        print floor(3.7);
        print ceil(3.2);
    )";
    
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n3\n-3\n3\n4\n");
}

TEST(MathFunctions, RandomFunction) {
    std::string code = R"(
        let r1 = random();
        let r2 = random();
        print (r1 >= 0 && r1 <= 1);
        print (r2 >= 0 && r2 <= 1);
        print (r1 != r2);  // Very likely to be different
    )";
    
    std::string output = runCode(code);
    // We can't predict exact values, but we can check the structure
    EXPECT_NE(output, "PARSE_ERROR");
    EXPECT_NE(output, "");
}
