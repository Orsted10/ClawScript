#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"
#include <sstream>
#include <iostream>

namespace {

class PrintCapture {
public:
    PrintCapture() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~PrintCapture() { std::cout.rdbuf(old); }
    std::string get() { return buffer.str(); }
private:
    std::stringstream buffer;
    std::streambuf* old;
};

std::string runCode(const std::string& source) {
    PrintCapture capture;
    
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) return "PARSE_ERROR";
    
    volt::Interpreter interpreter;
    try {
        interpreter.execute(statements);
        return capture.get();
    } catch (...) {
        return "RUNTIME_ERROR";
    }
}

} // anonymous namespace

// ========================================
// BREAK STATEMENT TESTS
// ========================================

TEST(EnhancedFeatures, BreakInWhileLoop) {
    std::string output = runCode(
        "let i = 0;"
        "while (true) {"
        "  print i;"
        "  i = i + 1;"
        "  if (i >= 3) break;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

TEST(EnhancedFeatures, BreakInForLoop) {
    std::string output = runCode(
        "for (let i = 0; i < 10; i = i + 1) {"
        "  if (i == 5) break;"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n3\n4\n");
}

TEST(EnhancedFeatures, BreakInNestedLoop) {
    std::string output = runCode(
        "for (let i = 0; i < 3; i = i + 1) {"
        "  for (let j = 0; j < 3; j = j + 1) {"
        "    if (j == 1) break;"
        "    print j;"
        "  }"
        "}"
    );
    EXPECT_EQ(output, "0\n0\n0\n");
}

// ========================================
// CONTINUE STATEMENT TESTS
// ========================================

TEST(EnhancedFeatures, ContinueInWhileLoop) {
    std::string output = runCode(
        "let i = 0;"
        "while (i < 5) {"
        "  i = i + 1;"
        "  if (i == 3) continue;"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "1\n2\n4\n5\n");
}

TEST(EnhancedFeatures, ContinueInForLoop) {
    std::string output = runCode(
        "for (let i = 0; i < 5; i = i + 1) {"
        "  if (i == 2) continue;"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n3\n4\n");
}

TEST(EnhancedFeatures, ContinueSkipsRest) {
    std::string output = runCode(
        "for (let i = 0; i < 3; i = i + 1) {"
        "  if (i == 1) continue;"
        "  print \"before\";"
        "  print i;"
        "  print \"after\";"
        "}"
    );
    EXPECT_EQ(output, "before\n0\nafter\nbefore\n2\nafter\n");
}

// ========================================
// COMPOUND ASSIGNMENT TESTS
// ========================================

TEST(EnhancedFeatures, PlusEqual) {
    std::string output = runCode(
        "let x = 10;"
        "x += 5;"
        "print x;"
    );
    EXPECT_EQ(output, "15\n");
}

TEST(EnhancedFeatures, MinusEqual) {
    std::string output = runCode(
        "let x = 20;"
        "x -= 8;"
        "print x;"
    );
    EXPECT_EQ(output, "12\n");
}

TEST(EnhancedFeatures, StarEqual) {
    std::string output = runCode(
        "let x = 3;"
        "x *= 4;"
        "print x;"
    );
    EXPECT_EQ(output, "12\n");
}

TEST(EnhancedFeatures, SlashEqual) {
    std::string output = runCode(
        "let x = 20;"
        "x /= 4;"
        "print x;"
    );
    EXPECT_EQ(output, "5\n");
}

TEST(EnhancedFeatures, CompoundWithStrings) {
    std::string output = runCode(
        "let s = \"Hello\";"
        "s += \" World\";"
        "print s;"
    );
    EXPECT_EQ(output, "Hello World\n");
}

TEST(EnhancedFeatures, CompoundInLoop) {
    std::string output = runCode(
        "let sum = 0;"
        "for (let i = 1; i <= 5; i += 1) {"
        "  sum += i;"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output, "15\n");
}

// ========================================
// INCREMENT/DECREMENT TESTS
// ========================================

TEST(EnhancedFeatures, PrefixIncrement) {
    std::string output = runCode(
        "let x = 5;"
        "print ++x;"
        "print x;"
    );
    EXPECT_EQ(output, "6\n6\n");
}

TEST(EnhancedFeatures, PostfixIncrement) {
    std::string output = runCode(
        "let x = 5;"
        "print x++;"
        "print x;"
    );
    EXPECT_EQ(output, "5\n6\n");
}

TEST(EnhancedFeatures, PrefixDecrement) {
    std::string output = runCode(
        "let x = 5;"
        "print --x;"
        "print x;"
    );
    EXPECT_EQ(output, "4\n4\n");
}

TEST(EnhancedFeatures, PostfixDecrement) {
    std::string output = runCode(
        "let x = 5;"
        "print x--;"
        "print x;"
    );
    EXPECT_EQ(output, "5\n4\n");
}

TEST(EnhancedFeatures, IncrementInForLoop) {
    std::string output = runCode(
        "for (let i = 0; i < 3; i++) {"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "0\n1\n2\n");
}

// ========================================
// TERNARY OPERATOR TESTS
// ========================================

TEST(EnhancedFeatures, BasicTernary) {
    std::string output = runCode(
        "let x = 10;"
        "let result = x > 5 ? \"big\" : \"small\";"
        "print result;"
    );
    EXPECT_EQ(output, "big\n");
}

TEST(EnhancedFeatures, TernaryWithNumbers) {
    std::string output = runCode(
        "let a = 5;"
        "let b = 10;"
        "let max = a > b ? a : b;"
        "print max;"
    );
    EXPECT_EQ(output, "10\n");
}

TEST(EnhancedFeatures, NestedTernary) {
    std::string output = runCode(
        "let x = 15;"
        "let result = x > 20 ? \"big\" : x > 10 ? \"medium\" : \"small\";"
        "print result;"
    );
    EXPECT_EQ(output, "medium\n");
}

TEST(EnhancedFeatures, TernaryInExpression) {
    std::string output = runCode(
        "let x = 5;"
        "print (x > 3 ? 100 : 50) + 10;"
    );
    EXPECT_EQ(output, "110\n");
}

// ========================================
// TYPE COERCION TESTS
// ========================================

TEST(EnhancedFeatures, StringPlusNumber) {
    std::string output = runCode(
        "print \"Count: \" + 42;"
    );
    EXPECT_EQ(output, "Count: 42\n");
}

TEST(EnhancedFeatures, NumberPlusString) {
    std::string output = runCode(
        "print 42 + \" items\";"
    );
    EXPECT_EQ(output, "42 items\n");
}

TEST(EnhancedFeatures, StringPlusEqualNumber) {
    std::string output = runCode(
        "let s = \"Score: \";"
        "s += 100;"
        "print s;"
    );
    EXPECT_EQ(output, "Score: 100\n");
}

// ========================================
// BUILT-IN FUNCTIONS TESTS
// ========================================

TEST(EnhancedFeatures, LenFunction) {
    std::string output = runCode(
        "print len(\"hello\");"
    );
    EXPECT_EQ(output, "5\n");
}

TEST(EnhancedFeatures, StrFunction) {
    std::string output = runCode(
        "print str(42);"
    );
    EXPECT_EQ(output, "42\n");
}

TEST(EnhancedFeatures, NumFunction) {
    std::string output = runCode(
        "let s = \"42\";"
        "let n = num(s);"
        "print n + 8;"
    );
    EXPECT_EQ(output, "50\n");
}

TEST(EnhancedFeatures, NumFromBool) {
    std::string output = runCode(
        "print num(true);"
        "print num(false);"
    );
    EXPECT_EQ(output, "1\n0\n");
}

TEST(EnhancedFeatures, ClockFunction) {
    std::string output = runCode(
        "let t = clock();"
        "print t > 0;"
    );
    EXPECT_EQ(output, "true\n");
}

TEST(EnhancedFeatures, UpperLowerFunctions) {
    std::string output = runCode(
        "print toUpper(\"hello\");"
        "print toLower(\"WORLD\");"
    );
    EXPECT_EQ(output, "HELLO\nworld\n");
}

TEST(EnhancedFeatures, TypeFunction) {
    std::string output = runCode(
        "print type(42);"
        "print type(\"hi\");"
        "print type(true);"
        "print type(nil);"
        "print type([]);"
        "fn f() {} print type(f);"
    );
    EXPECT_EQ(output, "number\nstring\nbool\nnil\narray\nfunction\n");
}

// ========================================
// COMBINED FEATURES TESTS
// ========================================

TEST(EnhancedFeatures, CountdownWithBreak) {
    std::string output = runCode(
        "for (let i = 10; i > 0; i--) {"
        "  if (i == 7) break;"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "10\n9\n8\n");
}

TEST(EnhancedFeatures, SkipEvensWithContinue) {
    std::string output = runCode(
        "for (let i = 0; i < 5; i++) {"
        "  if (i % 2 == 0) continue;"
        "  print i;"
        "}"
    );
    EXPECT_EQ(output, "1\n3\n");
}

TEST(EnhancedFeatures, TernaryWithCompound) {
    std::string output = runCode(
        "let x = 10;"
        "x += x > 5 ? 20 : 5;"
        "print x;"
    );
    EXPECT_EQ(output, "30\n");
}

TEST(EnhancedFeatures, ComplexExpression) {
    std::string output = runCode(
        "let sum = 0;"
        "for (let i = 1; i <= 10; i++) {"
        "  if (i % 2 == 0) continue;"
        "  sum += i;"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output, "25\n");
}
