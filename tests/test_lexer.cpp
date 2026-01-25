#include <gtest/gtest.h>
#include "lexer.h"

TEST(Lexer, Numbers) {
    volt::Lexer lexer("42 3.14");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3); // 2 numbers + EOF
    EXPECT_EQ(tokens[0].type, volt::TokenType::Number);
    EXPECT_EQ(tokens[0].lexeme, "42");
    EXPECT_EQ(tokens[1].lexeme, "3.14");
}

TEST(Lexer, Operators) {
    volt::Lexer lexer("+ - * / %");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens[0].type, volt::TokenType::Plus);
    EXPECT_EQ(tokens[1].type, volt::TokenType::Minus);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Star);
    EXPECT_EQ(tokens[3].type, volt::TokenType::Slash);
    EXPECT_EQ(tokens[4].type, volt::TokenType::Percent);
}

TEST(Lexer, Expression) {
    volt::Lexer lexer("1 + 2 * 3");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6);
}

TEST(Lexer, Keywords) {
    volt::Lexer lexer("let if else while fn return print");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::Let);
    EXPECT_EQ(tokens[1].type, volt::TokenType::If);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Else);
    EXPECT_EQ(tokens[3].type, volt::TokenType::While);
    EXPECT_EQ(tokens[4].type, volt::TokenType::Fn);
    EXPECT_EQ(tokens[5].type, volt::TokenType::Return);
    EXPECT_EQ(tokens[6].type, volt::TokenType::Print);
}

TEST(Lexer, Strings) {
    volt::Lexer lexer("\"hello\" \"world\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, volt::TokenType::String);
    // Lexeme now includes quotes
    EXPECT_EQ(tokens[0].lexeme, "\"hello\"");
    // stringValue holds processed content
    EXPECT_EQ(tokens[0].stringValue, "hello");
    
    EXPECT_EQ(tokens[1].lexeme, "\"world\"");
    EXPECT_EQ(tokens[1].stringValue, "world");
}

TEST(Lexer, Comparisons) {
    volt::Lexer lexer("== != < <= > >=");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::EqualEqual);
    EXPECT_EQ(tokens[0].lexeme, "==");
    
    EXPECT_EQ(tokens[1].type, volt::TokenType::BangEqual);
    EXPECT_EQ(tokens[1].lexeme, "!=");
    
    EXPECT_EQ(tokens[2].type, volt::TokenType::Less);
    EXPECT_EQ(tokens[2].lexeme, "<");
    
    EXPECT_EQ(tokens[3].type, volt::TokenType::LessEqual);
    EXPECT_EQ(tokens[3].lexeme, "<=");
    
    EXPECT_EQ(tokens[4].type, volt::TokenType::Greater);
    EXPECT_EQ(tokens[4].lexeme, ">");
    
    EXPECT_EQ(tokens[5].type, volt::TokenType::GreaterEqual);
    EXPECT_EQ(tokens[5].lexeme, ">=");
}

TEST(Lexer, BooleanLiterals) {
    volt::Lexer lexer("true false nil");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::True);
    EXPECT_EQ(tokens[1].type, volt::TokenType::False);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Nil);
}

TEST(Lexer, Comments) {
    volt::Lexer lexer("42 // this is a comment\n10");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3); // 42, 10, EOF
    EXPECT_EQ(tokens[0].lexeme, "42");
    EXPECT_EQ(tokens[1].lexeme, "10");
}

TEST(Lexer, Braces) {
    volt::Lexer lexer("{ }");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::LeftBrace);
    EXPECT_EQ(tokens[1].type, volt::TokenType::RightBrace);
}

TEST(Lexer, Semicolons) {
    volt::Lexer lexer("let x = 5;");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::Let);
    EXPECT_EQ(tokens[1].type, volt::TokenType::Identifier);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Equal);
    EXPECT_EQ(tokens[3].type, volt::TokenType::Number);
    EXPECT_EQ(tokens[4].type, volt::TokenType::Semicolon);
}

TEST(Lexer, ForLoop) {
    volt::Lexer lexer("for (let i = 0; i < 10; i = i + 1)");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::For);
    EXPECT_EQ(tokens[1].type, volt::TokenType::LeftParen);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Let);
}

TEST(Lexer, WhileLoop) {
    volt::Lexer lexer("while (x > 0)");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::While);
    EXPECT_EQ(tokens[1].type, volt::TokenType::LeftParen);
}

TEST(Lexer, PrintKeyword) {
    volt::Lexer lexer("print 42;");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::Print);
    EXPECT_EQ(tokens[1].type, volt::TokenType::Number);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Semicolon);
}
