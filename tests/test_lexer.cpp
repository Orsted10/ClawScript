#include <gtest/gtest.h>
#include "../src/lexer.h"

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
    EXPECT_EQ(tokens[4].type, volt::TokenType::Percent);
}

TEST(Lexer, Expression) {
    volt::Lexer lexer("1 + 2 * 3");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6);
}

TEST(Lexer, Keywords) {
    volt::Lexer lexer("let if else while fn return");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::Let);
    EXPECT_EQ(tokens[1].type, volt::TokenType::If);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Else);
    EXPECT_EQ(tokens[3].type, volt::TokenType::While);
    EXPECT_EQ(tokens[4].type, volt::TokenType::Fn);
    EXPECT_EQ(tokens[5].type, volt::TokenType::Return);
}

TEST(Lexer, Strings) {
    volt::Lexer lexer("\"hello\" \"world\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, volt::TokenType::String);
    EXPECT_EQ(tokens[0].lexeme, "hello");
    EXPECT_EQ(tokens[1].lexeme, "world");
}

TEST(Lexer, Comparisons) {
    volt::Lexer lexer("== != < <= > >=");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, volt::TokenType::EqualEqual);
    EXPECT_EQ(tokens[0].lexeme, "==");  // ← ADD THIS
    
    EXPECT_EQ(tokens[1].type, volt::TokenType::BangEqual);
    EXPECT_EQ(tokens[1].lexeme, "!=");  // ← ADD THIS
    
    EXPECT_EQ(tokens[2].type, volt::TokenType::Less);
    EXPECT_EQ(tokens[2].lexeme, "<");   // ← ADD THIS
    
    EXPECT_EQ(tokens[3].type, volt::TokenType::LessEqual);
    EXPECT_EQ(tokens[3].lexeme, "<=");  // ← ADD THIS
    
    EXPECT_EQ(tokens[4].type, volt::TokenType::Greater);
    EXPECT_EQ(tokens[4].lexeme, ">");   // ← ADD THIS
    
    EXPECT_EQ(tokens[5].type, volt::TokenType::GreaterEqual);
    EXPECT_EQ(tokens[5].lexeme, ">=");  // ← ADD THIS
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
