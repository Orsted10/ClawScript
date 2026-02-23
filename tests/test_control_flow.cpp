#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
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

// Control flow tests - Part 1
TEST(ControlFlow, SimpleIf1) {
    std::string code = "if (true) { print 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(ControlFlow, SimpleIf2) {
    std::string code = "if (false) { print 1; } else { print 2; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(ControlFlow, SimpleIf3) {
    std::string code = "let x = 5; if (x > 3) { print x; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(ControlFlow, SimpleIf4) {
    std::string code = "let y = 2; if (y < 5) { print y; } else { print 10; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(ControlFlow, SimpleIf5) {
    std::string code = "let z = 7; if (z == 7) { print 42; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n");
}

TEST(ControlFlow, SimpleIf6) {
    std::string code = "let a = 10; if (a != 5) { print a; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(ControlFlow, SimpleIf7) {
    std::string code = "let b = 0; if (b) { print 1; } else { print 0; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(ControlFlow, SimpleIf8) {
    std::string code = "let c = 1; if (c) { print 100; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(ControlFlow, SimpleIf9) {
    std::string code = "if (1 > 0) { print 99; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "99\n");
}

TEST(ControlFlow, SimpleIf10) {
    std::string code = "if (2 < 1) { print 1; } else { print 2; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(ControlFlow, WhileLoop1) {
    std::string code = "let i = 0; while (i < 3) { print i; i = i + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(ControlFlow, WhileLoop2) {
    std::string code = "let x = 5; while (x > 0) { print x; x = x - 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n4\n3\n2\n1\n");
}

TEST(ControlFlow, WhileLoop3) {
    std::string code = "let count = 0; while (count < 2) { print 42; count = count + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n42\n");
}

TEST(ControlFlow, WhileLoop4) {
    std::string code = "let n = 3; while (n >= 0) { print n; n = n - 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n2\n1\n0\n");
}

TEST(ControlFlow, WhileLoop5) {
    std::string code = "let x = 1; while (x <= 4) { print x * 2; x = x + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n6\n8\n");
}

TEST(ControlFlow, WhileLoop6) {
    std::string code = "let i = 10; while (i > 5) { print i; i = i - 2; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n8\n6\n");
}

TEST(ControlFlow, WhileLoop7) {
    std::string code = "let x = 0; while (x < 1) { print x; x = x + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(ControlFlow, WhileLoop8) {
    std::string code = "let y = 3; while (y != 0) { print y; y = y - 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n2\n1\n");
}

TEST(ControlFlow, WhileLoop9) {
    std::string code = "let z = 2; while (z < 6) { print z; z = z + 2; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n");
}

TEST(ControlFlow, WhileLoop10) {
    std::string code = "let counter = 1; while (counter * counter <= 16) { print counter; counter = counter + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n2\n3\n4\n");
}

TEST(ControlFlow, ForLoop1) {
    std::string code = "for (let i = 0; i < 3; i = i + 1) { print i; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(ControlFlow, ForLoop2) {
    std::string code = "for (let x = 1; x <= 3; x = x + 1) { print x * 2; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n6\n");
}

TEST(ControlFlow, ForLoop3) {
    std::string code = "for (let i = 5; i > 0; i = i - 1) { print i; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n4\n3\n2\n1\n");
}

TEST(ControlFlow, ForLoop4) {
    std::string code = "for (let j = 0; j < 2; j = j + 1) { print 42; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n42\n");
}

TEST(ControlFlow, ForLoop5) {
    std::string code = "for (let x = 10; x >= 8; x = x - 1) { print x; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n9\n8\n");
}

TEST(ControlFlow, ForLoop6) {
    std::string code = "for (let i = 2; i < 8; i = i + 2) { print i; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n6\n");
}

TEST(ControlFlow, ForLoop7) {
    std::string code = "for (let x = 1; x <= 4; x = x + 1) { print x * x; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n4\n9\n16\n");
}

TEST(ControlFlow, ForLoop8) {
    std::string code = "for (let i = 0; i < 1; i = i + 1) { print 999; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "999\n");
}

TEST(ControlFlow, ForLoop9) {
    std::string code = "for (let n = 3; n >= 1; n = n - 1) { print n * 3; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "9\n6\n3\n");
}

TEST(ControlFlow, ForLoop10) {
    std::string code = "for (let i = 1; i <= 5; i = i + 1) { if (i % 2 == 0) { print i; } }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n4\n");
}

TEST(ControlFlow, NestedLoops1) {
    std::string code = "for (let i = 0; i < 2; i = i + 1) { for (let j = 0; j < 2; j = j + 1) { print i * 10 + j; } }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n10\n11\n");
}

TEST(ControlFlow, NestedLoops2) {
    std::string code = "let x = 0; while (x < 2) { let y = 0; while (y < 2) { print x * 10 + y; y = y + 1; } x = x + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n10\n11\n");
}

TEST(ControlFlow, NestedLoops3) {
    std::string code = "for (let i = 1; i <= 2; i = i + 1) { for (let j = 1; j <= 2; j = j + 1) { print i * j; } }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n2\n2\n4\n");
}

TEST(ControlFlow, NestedLoops4) {
    std::string code = "let a = 1; while (a <= 2) { let b = 1; while (b <= 2) { print a + b; b = b + 1; } a = a + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n3\n3\n4\n");
}

TEST(ControlFlow, NestedLoops5) {
    std::string code = "for (let x = 0; x < 3; x = x + 1) { for (let y = 0; y < 1; y = y + 1) { print x; } }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(ControlFlow, BreakStatement1) {
    std::string code = "for (let i = 0; i < 10; i = i + 1) { if (i == 3) { break; } print i; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(ControlFlow, BreakStatement2) {
    std::string code = "let x = 0; while (x < 5) { if (x == 2) { break; } print x; x = x + 1; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n");
}

TEST(ControlFlow, ContinueStatement1) {
    std::string code = "for (let i = 0; i < 5; i = i + 1) { if (i == 2) { continue; } print i; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n1\n3\n4\n");
}

TEST(ControlFlow, ContinueStatement2) {
    std::string code = "let x = 0; while (x < 4) { x = x + 1; if (x == 2) { continue; } print x; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n3\n4\n");
}

TEST(ControlFlow, ComplexConditions1) {
    std::string code = "let x = 5; let y = 3; if (x > 3 && y < 5) { print 1; } else { print 0; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(ControlFlow, ComplexConditions2) {
    std::string code = "let a = 2; let b = 4; if (a < 3 || b > 5) { print 10; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(ControlFlow, ComplexConditions3) {
    std::string code = "let x = 1; if (!(x == 0)) { print 42; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n");
}

TEST(ControlFlow, ComplexConditions4) {
    std::string code = "let a = 3; let b = 3; if (a == b) { print 100; } else { print 200; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(ControlFlow, ComplexConditions5) {
    std::string code = "let x = 7; if (x >= 5 && x <= 10) { print 1; } else { print 0; }";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}
