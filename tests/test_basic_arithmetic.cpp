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

// Basic arithmetic tests - Part 1
TEST(BasicArithmetic, Addition1) {
    std::string code = "print 1 + 2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(BasicArithmetic, Addition2) {
    std::string code = "print 5 + 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, Addition3) {
    std::string code = "print 10 + 15;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "25\n");
}

TEST(BasicArithmetic, Addition4) {
    std::string code = "print 0 + 7;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(BasicArithmetic, Addition5) {
    std::string code = "print 100 + 200;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "300\n");
}

TEST(BasicArithmetic, Addition6) {
    std::string code = "print 42 + 58;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(BasicArithmetic, Addition7) {
    std::string code = "print 3 + 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(BasicArithmetic, Addition8) {
    std::string code = "print 9 + 1;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, Addition9) {
    std::string code = "print 12 + 8;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(BasicArithmetic, Addition10) {
    std::string code = "print 25 + 75;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(BasicArithmetic, Subtraction1) {
    std::string code = "print 5 - 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(BasicArithmetic, Subtraction2) {
    std::string code = "print 10 - 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, Subtraction3) {
    std::string code = "print 15 - 7;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, Subtraction4) {
    std::string code = "print 20 - 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "15\n");
}

TEST(BasicArithmetic, Subtraction5) {
    std::string code = "print 100 - 25;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "75\n");
}

TEST(BasicArithmetic, Subtraction6) {
    std::string code = "print 50 - 30;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(BasicArithmetic, Subtraction7) {
    std::string code = "print 7 - 7;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(BasicArithmetic, Subtraction8) {
    std::string code = "print 9 - 1;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, Subtraction9) {
    std::string code = "print 33 - 13;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(BasicArithmetic, Subtraction10) {
    std::string code = "print 1000 - 1;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "999\n");
}

TEST(BasicArithmetic, Multiplication1) {
    std::string code = "print 3 * 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "12\n");
}

TEST(BasicArithmetic, Multiplication2) {
    std::string code = "print 5 * 6;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "30\n");
}

TEST(BasicArithmetic, Multiplication3) {
    std::string code = "print 7 * 8;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "56\n");
}

TEST(BasicArithmetic, Multiplication4) {
    std::string code = "print 9 * 9;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "81\n");
}

TEST(BasicArithmetic, Multiplication5) {
    std::string code = "print 10 * 10;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(BasicArithmetic, Multiplication6) {
    std::string code = "print 0 * 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "0\n");
}

TEST(BasicArithmetic, Multiplication7) {
    std::string code = "print 1 * 100;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "100\n");
}

TEST(BasicArithmetic, Multiplication8) {
    std::string code = "print 12 * 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "60\n");
}

TEST(BasicArithmetic, Multiplication9) {
    std::string code = "print 2 * 15;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "30\n");
}

TEST(BasicArithmetic, Multiplication10) {
    std::string code = "print 8 * 7;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "56\n");
}

TEST(BasicArithmetic, Division1) {
    std::string code = "print 10 / 2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(BasicArithmetic, Division2) {
    std::string code = "print 15 / 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(BasicArithmetic, Division3) {
    std::string code = "print 20 / 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(BasicArithmetic, Division4) {
    std::string code = "print 100 / 10;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, Division5) {
    std::string code = "print 36 / 6;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, Division6) {
    std::string code = "print 49 / 7;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(BasicArithmetic, Division7) {
    std::string code = "print 64 / 8;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, Division8) {
    std::string code = "print 81 / 9;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "9\n");
}

TEST(BasicArithmetic, Division9) {
    std::string code = "print 144 / 12;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "12\n");
}

TEST(BasicArithmetic, Division10) {
    std::string code = "print 50 / 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, MixedOperations1) {
    std::string code = "print 2 + 3 * 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "14\n");
}

TEST(BasicArithmetic, MixedOperations2) {
    std::string code = "print (2 + 3) * 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "20\n");
}

TEST(BasicArithmetic, MixedOperations3) {
    std::string code = "print 10 - 2 * 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(BasicArithmetic, MixedOperations4) {
    std::string code = "print (10 - 2) * 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "24\n");
}

TEST(BasicArithmetic, MixedOperations5) {
    std::string code = "print 20 / 4 + 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, MixedOperations6) {
    std::string code = "print 20 / (4 + 1);";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(BasicArithmetic, MixedOperations7) {
    std::string code = "print 2 * 3 + 4 * 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "26\n");
}

TEST(BasicArithmetic, MixedOperations8) {
    std::string code = "print 1 + 2 * 3 + 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "11\n");
}

TEST(BasicArithmetic, MixedOperations9) {
    std::string code = "print 100 / 10 - 5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(BasicArithmetic, MixedOperations10) {
    std::string code = "print 8 * 2 - 6;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, FloatOperations1) {
    std::string code = "print 3.5 + 2.5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, FloatOperations2) {
    std::string code = "print 5.0 * 2.0;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, FloatOperations3) {
    std::string code = "print 10.0 / 3.0;";
    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(BasicArithmetic, FloatOperations4) {
    std::string code = "print 7.5 - 2.5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "5\n");
}

TEST(BasicArithmetic, FloatOperations5) {
    std::string code = "print 4.0 + 6.0;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, FloatOperations6) {
    std::string code = "print 1.5 * 4.0;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, FloatOperations7) {
    std::string code = "print 9.0 / 1.5;";
    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(BasicArithmetic, FloatOperations8) {
    std::string code = "print 12.5 - 2.5;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "10\n");
}

TEST(BasicArithmetic, FloatOperations9) {
    std::string code = "print 3.14 * 2.0;";
    std::string output = runCode(code);
    EXPECT_NE(output, "PARSE_ERROR");
}

TEST(BasicArithmetic, FloatOperations10) {
    std::string code = "print 16.0 / 4.0;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

// Bitwise operator tests
TEST(BasicArithmetic, BitwiseAnd) {
    std::string code = "print 5 & 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(BasicArithmetic, BitwiseOr) {
    std::string code = "print 5 | 2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(BasicArithmetic, BitwiseXor) {
    std::string code = "print 5 ^ 1;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(BasicArithmetic, BitwiseNot) {
    std::string code = "print ~0;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "-1\n");
}

TEST(BasicArithmetic, ShiftLeft) {
    std::string code = "print 1 << 3;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "8\n");
}

TEST(BasicArithmetic, ShiftRight) {
    std::string code = "print 8 >> 2;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(BasicArithmetic, BitwisePrecedence) {
    std::string code = "print 1 | 2 & 4;";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(BasicArithmetic, CompoundBitwiseAnd) {
    std::string code = R"(
        let x = 7;
        x &= 3;
        print x;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(BasicArithmetic, CompoundShiftLeft) {
    std::string code = R"(
        let x = 1;
        x <<= 4;
        print x;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(BasicArithmetic, ArrayIndexIncrement) {
    std::string code = R"(
        let a = [1];
        a[0]++;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "2\n");
}

TEST(BasicArithmetic, HashMapMemberDecrement) {
    std::string code = R"(
        let m = {"x": 5};
        m.x--;
        print m.x;
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}

TEST(BasicArithmetic, CompoundArrayIndexPlus) {
    std::string code = R"(
        let a = [5, 3];
        a[0] += 2;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "7\n");
}

TEST(BasicArithmetic, CompoundArrayIndexBitAnd) {
    std::string code = R"(
        let a = [5, 3];
        a[1] &= 1;
        print a[1];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "1\n");
}

TEST(BasicArithmetic, CompoundArrayIndexMinus) {
    std::string code = R"(
        let a = [10];
        a[0] -= 4;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, CompoundArrayIndexMultiply) {
    std::string code = R"(
        let a = [7];
        a[0] *= 3;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "21\n");
}

TEST(BasicArithmetic, CompoundArrayIndexDivide) {
    std::string code = R"(
        let a = [18];
        a[0] /= 6;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "3\n");
}

TEST(BasicArithmetic, CompoundArrayIndexXor) {
    std::string code = R"(
        let a = [5];
        a[0] ^= 3;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "6\n");
}

TEST(BasicArithmetic, CompoundArrayIndexShiftLeft) {
    std::string code = R"(
        let a = [2];
        a[0] <<= 3;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "16\n");
}

TEST(BasicArithmetic, CompoundArrayIndexShiftRight) {
    std::string code = R"(
        let a = [16];
        a[0] >>= 2;
        print a[0];
    )";
    std::string output = runCode(code);
    EXPECT_EQ(output, "4\n");
}
