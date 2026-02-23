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

// Variable operations tests - Part 1
TEST(VariableOperations, SimpleAssignment1) {
    std::string code = "let x = 5; print x;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(VariableOperations, SimpleAssignment2) {
    std::string code = "let y = 10; print y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(VariableOperations, SimpleAssignment3) {
    std::string code = "let z = 42; print z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n");
}

TEST(VariableOperations, SimpleAssignment4) {
    std::string code = "let a = 100; print a;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(VariableOperations, SimpleAssignment5) {
    std::string code = "let b = 0; print b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(VariableOperations, SimpleAssignment6) {
    std::string code = "let c = 999; print c;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "999\n");
}

TEST(VariableOperations, SimpleAssignment7) {
    std::string code = "let d = 7; print d;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(VariableOperations, SimpleAssignment8) {
    std::string code = "let e = 25; print e;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "25\n");
}

TEST(VariableOperations, SimpleAssignment9) {
    std::string code = "let f = 17; print f;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "17\n");
}

TEST(VariableOperations, SimpleAssignment10) {
    std::string code = "let g = 88; print g;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "88\n");
}

TEST(VariableOperations, VariableReassignment1) {
    std::string code = "let x = 5; x = 10; print x;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(VariableOperations, VariableReassignment2) {
    std::string code = "let y = 100; y = 200; print y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "200\n");
}

TEST(VariableOperations, VariableReassignment3) {
    std::string code = "let z = 1; z = 2; print z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(VariableOperations, VariableReassignment4) {
    std::string code = "let a = 50; a = 75; print a;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "75\n");
}

TEST(VariableOperations, VariableReassignment5) {
    std::string code = "let b = 33; b = 66; print b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "66\n");
}

TEST(VariableOperations, VariableReassignment6) {
    std::string code = "let c = 12; c = 34; print c;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "34\n");
}

TEST(VariableOperations, VariableReassignment7) {
    std::string code = "let d = 89; d = 90; print d;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "90\n");
}

TEST(VariableOperations, VariableReassignment8) {
    std::string code = "let e = 4; e = 44; print e;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "44\n");
}

TEST(VariableOperations, VariableReassignment9) {
    std::string code = "let f = 7; f = 77; print f;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "77\n");
}

TEST(VariableOperations, VariableReassignment10) {
    std::string code = "let g = 9; g = 99; print g;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "99\n");
}

TEST(VariableOperations, VariableArithmetic1) {
    std::string code = "let x = 5; x = x + 3; print x;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(VariableOperations, VariableArithmetic2) {
    std::string code = "let y = 10; y = y - 4; print y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(VariableOperations, VariableArithmetic3) {
    std::string code = "let z = 3; z = z * 5; print z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "15\n");
}

TEST(VariableOperations, VariableArithmetic4) {
    std::string code = "let a = 20; a = a / 4; print a;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(VariableOperations, VariableArithmetic5) {
    std::string code = "let b = 7; b = b + 13; print b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(VariableOperations, VariableArithmetic6) {
    std::string code = "let c = 100; c = c - 25; print c;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "75\n");
}

TEST(VariableOperations, VariableArithmetic7) {
    std::string code = "let d = 6; d = d * 7; print d;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "42\n");
}

TEST(VariableOperations, VariableArithmetic8) {
    std::string code = "let e = 36; e = e / 6; print e;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(VariableOperations, VariableArithmetic9) {
    std::string code = "let f = 15; f = f + 25; print f;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "40\n");
}

TEST(VariableOperations, VariableArithmetic10) {
    std::string code = "let g = 50; g = g - 10; print g;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "40\n");
}

TEST(VariableOperations, MultipleVariables1) {
    std::string code = "let x = 5; let y = 10; print x + y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "15\n");
}

TEST(VariableOperations, MultipleVariables2) {
    std::string code = "let a = 7; let b = 3; print a * b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "21\n");
}

TEST(VariableOperations, MultipleVariables3) {
    std::string code = "let x = 20; let y = 5; print x / y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(VariableOperations, MultipleVariables4) {
    std::string code = "let p = 15; let q = 8; print p - q;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(VariableOperations, MultipleVariables5) {
    std::string code = "let m = 100; let n = 50; print m + n;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "150\n");
}

TEST(VariableOperations, MultipleVariables6) {
    std::string code = "let i = 4; let j = 6; let k = 2; print i + j * k;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(VariableOperations, MultipleVariables7) {
    std::string code = "let x = 12; let y = 3; let z = 4; print (x / y) * z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(VariableOperations, MultipleVariables8) {
    std::string code = "let a = 25; let b = 5; let c = 2; print a - b * c;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "15\n");
}

TEST(VariableOperations, MultipleVariables9) {
    std::string code = "let p = 8; let q = 4; let r = 2; print (p + q) / r;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(VariableOperations, MultipleVariables10) {
    std::string code = "let x = 18; let y = 6; let z = 3; print x / y - z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(VariableOperations, VariableWithExpressions1) {
    std::string code = "let x = 2 + 3; print x;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(VariableOperations, VariableWithExpressions2) {
    std::string code = "let y = 10 - 4; print y;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(VariableOperations, VariableWithExpressions3) {
    std::string code = "let z = 7 * 8; print z;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "56\n");
}

TEST(VariableOperations, VariableWithExpressions4) {
    std::string code = "let a = 36 / 6; print a;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(VariableOperations, VariableWithExpressions5) {
    std::string code = "let b = (5 + 3) * 2; print b;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(VariableOperations, VariableWithExpressions6) {
    std::string code = "let c = 100 / (5 + 5); print c;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(VariableOperations, VariableWithExpressions7) {
    std::string code = "let d = 2 * 3 + 4 * 5; print d;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "26\n");
}

TEST(VariableOperations, VariableWithExpressions8) {
    std::string code = "let e = (10 - 6) * (3 + 2); print e;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(VariableOperations, VariableWithExpressions9) {
    std::string code = "let f = 15 / 3 + 7 * 2; print f;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "19\n");
}

TEST(VariableOperations, VariableWithExpressions10) {
    std::string code = "let g = 8 * 2 - 6 / 3; print g;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "14\n");
}
