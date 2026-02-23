#include <gtest/gtest.h>
#include "lexer.h"

TEST(Lexer, Numbers) {
    claw::Lexer lexer("42 3.14");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3); // 2 numbers + EOF
    EXPECT_EQ(tokens[0].type, claw::TokenType::Number);
    EXPECT_EQ(tokens[0].lexeme, "42");
    EXPECT_EQ(tokens[1].lexeme, "3.14");
}

TEST(Lexer, Operators) {
    claw::Lexer lexer("+ - * / %");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens[0].type, claw::TokenType::Plus);
    EXPECT_EQ(tokens[1].type, claw::TokenType::Minus);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Star);
    EXPECT_EQ(tokens[3].type, claw::TokenType::Slash);
    EXPECT_EQ(tokens[4].type, claw::TokenType::Percent);
}

TEST(Lexer, Expression) {
    claw::Lexer lexer("1 + 2 * 3");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6);
}

TEST(Lexer, Keywords) {
    claw::Lexer lexer("let if else while fn return print");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::Let);
    EXPECT_EQ(tokens[1].type, claw::TokenType::If);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Else);
    EXPECT_EQ(tokens[3].type, claw::TokenType::While);
    EXPECT_EQ(tokens[4].type, claw::TokenType::Fn);
    EXPECT_EQ(tokens[5].type, claw::TokenType::Return);
    EXPECT_EQ(tokens[6].type, claw::TokenType::Print);
}

TEST(Lexer, Strings) {
    claw::Lexer lexer("\"hello\" \"world\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, claw::TokenType::String);
    // Lexeme now includes quotes
    EXPECT_EQ(tokens[0].lexeme, "\"hello\"");
    // stringValue holds processed content
    EXPECT_EQ(tokens[0].stringValue, "hello");
    
    EXPECT_EQ(tokens[1].lexeme, "\"world\"");
    EXPECT_EQ(tokens[1].stringValue, "world");
}

TEST(Lexer, Comparisons) {
    claw::Lexer lexer("== != < <= > >=");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::EqualEqual);
    EXPECT_EQ(tokens[0].lexeme, "==");
    
    EXPECT_EQ(tokens[1].type, claw::TokenType::BangEqual);
    EXPECT_EQ(tokens[1].lexeme, "!=");
    
    EXPECT_EQ(tokens[2].type, claw::TokenType::Less);
    EXPECT_EQ(tokens[2].lexeme, "<");
    
    EXPECT_EQ(tokens[3].type, claw::TokenType::LessEqual);
    EXPECT_EQ(tokens[3].lexeme, "<=");
    
    EXPECT_EQ(tokens[4].type, claw::TokenType::Greater);
    EXPECT_EQ(tokens[4].lexeme, ">");
    
    EXPECT_EQ(tokens[5].type, claw::TokenType::GreaterEqual);
    EXPECT_EQ(tokens[5].lexeme, ">=");
}

TEST(Lexer, BooleanLiterals) {
    claw::Lexer lexer("true false nil");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::True);
    EXPECT_EQ(tokens[1].type, claw::TokenType::False);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Nil);
}

TEST(Lexer, Comments) {
    claw::Lexer lexer("42 // this is a comment\n10");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3); // 42, 10, EOF
    EXPECT_EQ(tokens[0].lexeme, "42");
    EXPECT_EQ(tokens[1].lexeme, "10");
}

TEST(Lexer, Braces) {
    claw::Lexer lexer("{ }");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::LeftBrace);
    EXPECT_EQ(tokens[1].type, claw::TokenType::RightBrace);
}

TEST(Lexer, Semicolons) {
    claw::Lexer lexer("let x = 5;");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::Let);
    EXPECT_EQ(tokens[1].type, claw::TokenType::Identifier);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Equal);
    EXPECT_EQ(tokens[3].type, claw::TokenType::Number);
    EXPECT_EQ(tokens[4].type, claw::TokenType::Semicolon);
}

TEST(Lexer, ForLoop) {
    claw::Lexer lexer("for (let i = 0; i < 10; i = i + 1)");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::For);
    EXPECT_EQ(tokens[1].type, claw::TokenType::LeftParen);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Let);
}

TEST(Lexer, WhileLoop) {
    claw::Lexer lexer("while (x > 0)");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::While);
    EXPECT_EQ(tokens[1].type, claw::TokenType::LeftParen);
}

TEST(Lexer, PrintKeyword) {
    claw::Lexer lexer("print 42;");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, claw::TokenType::Print);
    EXPECT_EQ(tokens[1].type, claw::TokenType::Number);
    EXPECT_EQ(tokens[2].type, claw::TokenType::Semicolon);
}
