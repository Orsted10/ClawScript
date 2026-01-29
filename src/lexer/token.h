#pragma once
#include <string_view>
#include <string>

namespace volt {

enum class TokenType {
    // Literals
    Number, String, Identifier,
    
    // Keywords
    Let, If, Else, While, For, Run, Until, Fn, Return,
    True, False, Nil, Print,
    Break, Continue, // Loop control
    
    // Operators
    Plus, Minus, Star, Slash, Percent,
    Equal, EqualEqual, Bang, BangEqual,
    Less, LessEqual, Greater, GreaterEqual,
    And, Or,
    
    // Compound assignment operators
    PlusEqual, MinusEqual, StarEqual, SlashEqual,
    
    // Increment/Decrement
    PlusPlus, MinusMinus,
    
    // Ternary operator
    Question, Colon,
    
    // Punctuation
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftBracket, RightBracket,  // Added for arrays [  ]
    Semicolon, Comma,
    Dot,  // Added for array.length, array.push(), etc.
    
    // Special
    Eof, Error
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    int column;
    std::string stringValue; // For processed string literals (with escape sequences)
    
    Token(TokenType t, std::string_view lex, int ln, int col = 1)
        : type(t), lexeme(lex), line(ln), column(col) {}
    
    // Constructor for string tokens with processed value
    Token(TokenType t, std::string_view lex, int ln, int col, std::string strVal)
        : type(t), lexeme(lex), line(ln), column(col), stringValue(std::move(strVal)) {}
};

const char* tokenName(TokenType type);

} // namespace volt
