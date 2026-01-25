#pragma once
#include "token.h"
#include <string_view>
#include <vector>
#include <unordered_map>

namespace volt {

class Lexer {
public:
    explicit Lexer(std::string_view source);
    
    std::vector<Token> tokenize();
    
private:
    Token scanToken();
    Token number();
    Token identifier();
    Token string();
    
    void skipWhitespace();
    
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    bool isAtEnd() const;
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    
    std::string_view source_;
    size_t current_ = 0;
    size_t start_ = 0;
    int line_ = 1;
    int column_ = 1;
    int startColumn_ = 1;
    
    static const std::unordered_map<std::string_view, TokenType> keywords_;
};

} // namespace volt
