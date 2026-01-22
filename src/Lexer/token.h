#pragma once
#include <string>
#include <string_view>

namespace volt {

enum class TokenType {
    // Literals
    Number, String, Identifier,
    
    // Keywords
    Let, If, Else, While, For, Run, Until, Fn, Return,
    True, False, Nil, Print,
    
    // Operators
    Plus, Minus, Star, Slash, Percent,
    Equal, EqualEqual, Bang, BangEqual,
    Less, LessEqual, Greater, GreaterEqual,
    And, Or,
    
    // Punctuation
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Semicolon, Comma,
    
    // Special
    Eof, Error
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    
    Token(TokenType t, std::string_view lex, int ln)
        : type(t), lexeme(lex), line(ln) {}
};

const char* tokenName(TokenType type);

} // namespace volt
