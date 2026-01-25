#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "value.h"

// Helper function
std::string evalExpr(const std::string& source) {
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto ast = parser.parseExpression();
    
    if (!ast || parser.hadError()) return "ERROR";
    
    volt::Interpreter interpreter;
    try {
        volt::Value result = interpreter.evaluate(ast.get());
        return volt::valueToString(result);
    } catch (...) {
        return "ERROR";
    }
}

// ===== LITERALS =====
TEST(Evaluator, Numbers) {
    EXPECT_EQ(evalExpr("42"), "42");
    EXPECT_EQ(evalExpr("3.14"), "3.14");
}

TEST(Evaluator, Strings) {
    EXPECT_EQ(evalExpr("\"hello\""), "hello");
    EXPECT_EQ(evalExpr("\"world\""), "world");
}

TEST(Evaluator, Booleans) {
    EXPECT_EQ(evalExpr("true"), "true");
    EXPECT_EQ(evalExpr("false"), "false");
}

TEST(Evaluator, Nil) {
    EXPECT_EQ(evalExpr("nil"), "nil");
}

// ===== ARITHMETIC =====
TEST(Evaluator, Addition) {
    EXPECT_EQ(evalExpr("1 + 2"), "3");
    EXPECT_EQ(evalExpr("10 + 5"), "15");
}

TEST(Evaluator, Subtraction) {
    EXPECT_EQ(evalExpr("10 - 3"), "7");
    EXPECT_EQ(evalExpr("5 - 10"), "-5");
}

TEST(Evaluator, Multiplication) {
    EXPECT_EQ(evalExpr("2 * 3"), "6");
    EXPECT_EQ(evalExpr("7 * 8"), "56");
}

TEST(Evaluator, Division) {
    EXPECT_EQ(evalExpr("10 / 2"), "5");
    EXPECT_EQ(evalExpr("15 / 3"), "5");
}

TEST(Evaluator, Modulo) {
    EXPECT_EQ(evalExpr("10 % 3"), "1");
    EXPECT_EQ(evalExpr("20 % 7"), "6");
}

TEST(Evaluator, Precedence) {
    EXPECT_EQ(evalExpr("1 + 2 * 3"), "7");
    EXPECT_EQ(evalExpr("10 - 4 / 2"), "8");
    EXPECT_EQ(evalExpr("2 * 3 + 4 * 5"), "26");
}

TEST(Evaluator, Parentheses) {
    EXPECT_EQ(evalExpr("(1 + 2) * 3"), "9");
    EXPECT_EQ(evalExpr("(10 + 5) / 3"), "5");
}

// ===== UNARY =====
TEST(Evaluator, Negation) {
    EXPECT_EQ(evalExpr("-5"), "-5");
    EXPECT_EQ(evalExpr("-(10 + 5)"), "-15");
}

TEST(Evaluator, LogicalNot) {
    EXPECT_EQ(evalExpr("!true"), "false");
    EXPECT_EQ(evalExpr("!false"), "true");
    EXPECT_EQ(evalExpr("!!true"), "true");
}

// ===== COMPARISON =====
TEST(Evaluator, LessThan) {
    EXPECT_EQ(evalExpr("1 < 2"), "true");
    EXPECT_EQ(evalExpr("5 < 3"), "false");
}

TEST(Evaluator, LessEqual) {
    EXPECT_EQ(evalExpr("1 <= 2"), "true");
    EXPECT_EQ(evalExpr("5 <= 5"), "true");
    EXPECT_EQ(evalExpr("10 <= 5"), "false");
}

TEST(Evaluator, GreaterThan) {
    EXPECT_EQ(evalExpr("5 > 3"), "true");
    EXPECT_EQ(evalExpr("2 > 10"), "false");
}

TEST(Evaluator, GreaterEqual) {
    EXPECT_EQ(evalExpr("10 >= 5"), "true");
    EXPECT_EQ(evalExpr("5 >= 5"), "true");
}

TEST(Evaluator, Equality) {
    EXPECT_EQ(evalExpr("5 == 5"), "true");
    EXPECT_EQ(evalExpr("5 == 3"), "false");
    EXPECT_EQ(evalExpr("true == true"), "true");
    EXPECT_EQ(evalExpr("true == false"), "false");
}

TEST(Evaluator, Inequality) {
    EXPECT_EQ(evalExpr("5 != 3"), "true");
    EXPECT_EQ(evalExpr("5 != 5"), "false");
}

// ===== LOGICAL =====
TEST(Evaluator, LogicalAnd) {
    EXPECT_EQ(evalExpr("true && true"), "true");
    EXPECT_EQ(evalExpr("true && false"), "false");
    EXPECT_EQ(evalExpr("false && true"), "false");
}

TEST(Evaluator, LogicalOr) {
    EXPECT_EQ(evalExpr("true || false"), "true");
    EXPECT_EQ(evalExpr("false || true"), "true");
    EXPECT_EQ(evalExpr("false || false"), "false");
}

// ===== STRING OPERATIONS =====
TEST(Evaluator, StringConcatenation) {
    EXPECT_EQ(evalExpr("\"hello\" + \" world\""), "hello world");
    EXPECT_EQ(evalExpr("\"foo\" + \"bar\""), "foobar");
}

// ===== COMPLEX EXPRESSIONS =====
TEST(Evaluator, ComplexArithmetic) {
    EXPECT_EQ(evalExpr("(5 + 3) * 2 - 10 / 5"), "14");
    EXPECT_EQ(evalExpr("((10 + 5) * 2) / 3"), "10");
}

TEST(Evaluator, ComplexLogic) {
    EXPECT_EQ(evalExpr("(5 > 3) && (10 < 20)"), "true");
    EXPECT_EQ(evalExpr("(5 > 10) || (3 < 7)"), "true");
    EXPECT_EQ(evalExpr("!(5 == 3) && (10 != 5)"), "true");
}

// ===== ERROR CASES =====
TEST(Evaluator, DivisionByZero) {
    EXPECT_EQ(evalExpr("10 / 0"), "ERROR");
}

TEST(Evaluator, TypeCoercion) {
    // string + number now coerces to string
    EXPECT_EQ(evalExpr("5 + \"hello\""), "5hello");
    EXPECT_EQ(evalExpr("\"value: \" + 42"), "value: 42");
    // subtraction still requires numbers
    EXPECT_EQ(evalExpr("\"hello\" - 5"), "ERROR");
}
