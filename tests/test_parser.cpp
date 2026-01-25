#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Helper function for expressions
std::string parseExpr(const std::string& source) {
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto ast = parser.parseExpression();
    return ast ? volt::printAST(ast.get()) : "";
}

// ========================================
// EXPRESSION TESTS (from Milestone 3)
// ========================================

TEST(Parser, Numbers) {
    EXPECT_EQ(parseExpr("42"), "42.000000");
    EXPECT_EQ(parseExpr("3.14"), "3.140000");
}

TEST(Parser, Strings) {
    EXPECT_EQ(parseExpr("\"hello\""), "\"hello\"");
}

TEST(Parser, Booleans) {
    EXPECT_EQ(parseExpr("true"), "true");
    EXPECT_EQ(parseExpr("false"), "false");
    EXPECT_EQ(parseExpr("nil"), "nil");
}

TEST(Parser, Variables) {
    EXPECT_EQ(parseExpr("x"), "x");
    EXPECT_EQ(parseExpr("myVar"), "myVar");
}

TEST(Parser, Addition) {
    EXPECT_EQ(parseExpr("1 + 2"), "(+ 1.000000 2.000000)");
}

TEST(Parser, Precedence) {
    EXPECT_EQ(parseExpr("1 + 2 * 3"), "(+ 1.000000 (* 2.000000 3.000000))");
    EXPECT_EQ(parseExpr("1 * 2 + 3"), "(+ (* 1.000000 2.000000) 3.000000)");
}

TEST(Parser, Parentheses) {
    EXPECT_EQ(parseExpr("(1 + 2) * 3"), "(* (group (+ 1.000000 2.000000)) 3.000000)");
}

TEST(Parser, Unary) {
    EXPECT_EQ(parseExpr("-10"), "(- 10.000000)");
    EXPECT_EQ(parseExpr("!true"), "(! true)");
}

TEST(Parser, Comparison) {
    EXPECT_EQ(parseExpr("1 < 2"), "(< 1.000000 2.000000)");
    EXPECT_EQ(parseExpr("3 >= 4"), "(>= 3.000000 4.000000)");
    EXPECT_EQ(parseExpr("5 == 5"), "(== 5.000000 5.000000)");
}

TEST(Parser, Logical) {
    EXPECT_EQ(parseExpr("true && false"), "(&& true false)");
    EXPECT_EQ(parseExpr("a || b"), "(|| a b)");
}

TEST(Parser, Assignment) {
    EXPECT_EQ(parseExpr("x = 10"), "(= x 10.000000)");
    EXPECT_EQ(parseExpr("y = 5 + 3"), "(= y (+ 5.000000 3.000000))");
}

TEST(Parser, ComplexExpression) {
    auto result = parseExpr("a = (b + c) * d - e / f");
    EXPECT_TRUE(result.find("=") != std::string::npos);
    EXPECT_TRUE(result.find("-") != std::string::npos);
}

TEST(Parser, FunctionCall) {
    EXPECT_EQ(parseExpr("foo()"), "(call foo)");
    EXPECT_EQ(parseExpr("add(1, 2)"), "(call add 1.000000 2.000000)");
}

// ========================================
// STATEMENT TESTS (NEW for Milestone 5)
// ========================================

TEST(Parser, PrintStatement) {
    volt::Lexer lexer("print 42;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    EXPECT_TRUE(dynamic_cast<volt::PrintStmt*>(statements[0].get()) != nullptr);
}

TEST(Parser, LetStatement) {
    volt::Lexer lexer("let x = 10;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* letStmt = dynamic_cast<volt::LetStmt*>(statements[0].get());
    EXPECT_TRUE(letStmt != nullptr);
    EXPECT_EQ(letStmt->name, "x");
}

TEST(Parser, LetWithoutInitializer) {
    volt::Lexer lexer("let x;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* letStmt = dynamic_cast<volt::LetStmt*>(statements[0].get());
    EXPECT_TRUE(letStmt != nullptr);
    EXPECT_TRUE(letStmt->initializer == nullptr);
}

TEST(Parser, BlockStatement) {
    volt::Lexer lexer("{ print 1; print 2; }");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* blockStmt = dynamic_cast<volt::BlockStmt*>(statements[0].get());
    EXPECT_TRUE(blockStmt != nullptr);
    EXPECT_EQ(blockStmt->statements.size(), 2);
}

TEST(Parser, IfStatement) {
    volt::Lexer lexer("if (true) print 1;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* ifStmt = dynamic_cast<volt::IfStmt*>(statements[0].get());
    EXPECT_TRUE(ifStmt != nullptr);
    EXPECT_TRUE(ifStmt->condition != nullptr);
    EXPECT_TRUE(ifStmt->thenBranch != nullptr);
    EXPECT_TRUE(ifStmt->elseBranch == nullptr);
}

TEST(Parser, IfElseStatement) {
    volt::Lexer lexer("if (false) print 1; else print 2;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* ifStmt = dynamic_cast<volt::IfStmt*>(statements[0].get());
    EXPECT_TRUE(ifStmt != nullptr);
    EXPECT_TRUE(ifStmt->elseBranch != nullptr);
}

TEST(Parser, WhileStatement) {
    volt::Lexer lexer("while (x < 10) print x;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* whileStmt = dynamic_cast<volt::WhileStmt*>(statements[0].get());
    EXPECT_TRUE(whileStmt != nullptr);
    EXPECT_TRUE(whileStmt->condition != nullptr);
    EXPECT_TRUE(whileStmt->body != nullptr);
}

TEST(Parser, ForStatement) {
    volt::Lexer lexer("for (let i = 0; i < 10; i = i + 1) print i;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* forStmt = dynamic_cast<volt::ForStmt*>(statements[0].get());
    EXPECT_TRUE(forStmt != nullptr);
    EXPECT_TRUE(forStmt->initializer != nullptr);
    EXPECT_TRUE(forStmt->condition != nullptr);
    EXPECT_TRUE(forStmt->increment != nullptr);
    EXPECT_TRUE(forStmt->body != nullptr);
}

TEST(Parser, ForStatementPartial) {
    volt::Lexer lexer("for (; i < 10;) print i;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* forStmt = dynamic_cast<volt::ForStmt*>(statements[0].get());
    EXPECT_TRUE(forStmt != nullptr);
    EXPECT_TRUE(forStmt->initializer == nullptr);
    EXPECT_TRUE(forStmt->condition != nullptr);
    EXPECT_TRUE(forStmt->increment == nullptr);
}

TEST(Parser, MultipleStatements) {
    volt::Lexer lexer("let x = 5; print x; x = 10;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    EXPECT_EQ(statements.size(), 3);
}

TEST(Parser, NestedBlocks) {
    volt::Lexer lexer("{ { print 1; } }");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* outer = dynamic_cast<volt::BlockStmt*>(statements[0].get());
    EXPECT_TRUE(outer != nullptr);
    ASSERT_EQ(outer->statements.size(), 1);
    
    auto* inner = dynamic_cast<volt::BlockStmt*>(outer->statements[0].get());
    EXPECT_TRUE(inner != nullptr);
}

TEST(Parser, ExpressionStatement) {
    volt::Lexer lexer("1 + 2;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_FALSE(parser.hadError());
    ASSERT_EQ(statements.size(), 1);
    
    auto* exprStmt = dynamic_cast<volt::ExprStmt*>(statements[0].get());
    EXPECT_TRUE(exprStmt != nullptr);
}

// ========================================
// ERROR HANDLING TESTS
// ========================================

TEST(Parser, MissingSemicolon) {
    volt::Lexer lexer("let x = 5");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_TRUE(parser.hadError());
}

TEST(Parser, MissingClosingBrace) {
    volt::Lexer lexer("{ print 1;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_TRUE(parser.hadError());
}

TEST(Parser, MissingConditionParen) {
    volt::Lexer lexer("if true print 1;");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    EXPECT_TRUE(parser.hadError());
}
