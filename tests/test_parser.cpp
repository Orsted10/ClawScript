#include <gtest/gtest.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/ast.h"

// Helper function
std::string parse(const std::string& source) {
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto ast = parser.parse();
    return ast ? volt::printAST(ast.get()) : "";
}

TEST(Parser, Numbers) {
    EXPECT_EQ(parse("42"), "42.000000");
    EXPECT_EQ(parse("3.14"), "3.140000");
}

TEST(Parser, Strings) {
    EXPECT_EQ(parse("\"hello\""), "\"hello\"");
}

TEST(Parser, Booleans) {
    EXPECT_EQ(parse("true"), "true");
    EXPECT_EQ(parse("false"), "false");
    EXPECT_EQ(parse("nil"), "nil");
}

TEST(Parser, Variables) {
    EXPECT_EQ(parse("x"), "x");
    EXPECT_EQ(parse("myVar"), "myVar");
}

TEST(Parser, Addition) {
    EXPECT_EQ(parse("1 + 2"), "(+ 1.000000 2.000000)");
}

TEST(Parser, Precedence) {
    EXPECT_EQ(parse("1 + 2 * 3"), "(+ 1.000000 (* 2.000000 3.000000))");
    EXPECT_EQ(parse("1 * 2 + 3"), "(+ (* 1.000000 2.000000) 3.000000)");
}

TEST(Parser, Parentheses) {
    EXPECT_EQ(parse("(1 + 2) * 3"), "(* (group (+ 1.000000 2.000000)) 3.000000)");
}

TEST(Parser, Unary) {
    EXPECT_EQ(parse("-10"), "(- 10.000000)");
    EXPECT_EQ(parse("!true"), "(! true)");
}

TEST(Parser, Comparison) {
    EXPECT_EQ(parse("1 < 2"), "(< 1.000000 2.000000)");
    EXPECT_EQ(parse("3 >= 4"), "(>= 3.000000 4.000000)");
    EXPECT_EQ(parse("5 == 5"), "(== 5.000000 5.000000)");
}

TEST(Parser, Logical) {
    EXPECT_EQ(parse("true && false"), "(&& true false)");
    EXPECT_EQ(parse("a || b"), "(|| a b)");
}

TEST(Parser, Assignment) {
    EXPECT_EQ(parse("x = 10"), "(= x 10.000000)");
    EXPECT_EQ(parse("y = 5 + 3"), "(= y (+ 5.000000 3.000000))");
}

TEST(Parser, ComplexExpression) {
    auto result = parse("a = (b + c) * d - e / f");
    EXPECT_TRUE(result.find("=") != std::string::npos);
    EXPECT_TRUE(result.find("-") != std::string::npos);
}

TEST(Parser, FunctionCall) {
    EXPECT_EQ(parse("foo()"), "(call foo)");
    EXPECT_EQ(parse("add(1, 2)"), "(call add 1.000000 2.000000)");
}
