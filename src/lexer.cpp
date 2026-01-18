#include "lexer.h"
#include <iostream>

namespace volt {

const std::unordered_map<std::string_view, TokenType> Lexer::keywords_ = {
    {"let", TokenType::Let},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"fn", TokenType::Fn},
    {"return", TokenType::Return},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"nil", TokenType::Nil},
};

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;
        
        start_ = current_;
        tokens.push_back(scanToken());
    }
    
    tokens.push_back(Token(TokenType::Eof, "", line_));
    return tokens;
}

Token Lexer::scanToken() {
    start_ = current_;
    char c = advance();
    
    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();
    if (c == '"') return string();
    
    switch (c) {
        case '(': return Token(TokenType::LeftParen, "(", line_);
        case ')': return Token(TokenType::RightParen, ")", line_);
        case '{': return Token(TokenType::LeftBrace, "{", line_);
        case '}': return Token(TokenType::RightBrace, "}", line_);
        case ';': return Token(TokenType::Semicolon, ";", line_);
        case ',': return Token(TokenType::Comma, ",", line_);
        case '+': return Token(TokenType::Plus, "+", line_);
        case '-': return Token(TokenType::Minus, "-", line_);
        case '*': return Token(TokenType::Star, "*", line_);
        case '/': return Token(TokenType::Slash, "/", line_);
        case '%': return Token(TokenType::Percent, "%", line_);
        
        case '=':
            if (match('=')) {
                return Token(TokenType::EqualEqual, "==", line_);
            }
            return Token(TokenType::Equal, "=", line_);
        
        case '!':
            if (match('=')) {
                return Token(TokenType::BangEqual, "!=", line_);
            }
            return Token(TokenType::Bang, "!", line_);
        
        case '<':
            if (match('=')) {
                return Token(TokenType::LessEqual, "<=", line_);
            }
            return Token(TokenType::Less, "<", line_);
        
        case '>':
            if (match('=')) {
                return Token(TokenType::GreaterEqual, ">=", line_);
            }
            return Token(TokenType::Greater, ">", line_);
        
        case '&':
            if (match('&')) {
                return Token(TokenType::And, "&&", line_);
            }
            break;
        
        case '|':
            if (match('|')) {
                return Token(TokenType::Or, "||", line_);
            }
            break;
    }
    
    return Token(TokenType::Error, source_.substr(start_, 1), line_);
}

Token Lexer::number() {
    while (isDigit(peek())) advance();
    
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }
    
    return Token(TokenType::Number, 
                 source_.substr(start_, current_ - start_), 
                 line_);
}

Token Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();
    
    std::string_view text = source_.substr(start_, current_ - start_);
    
    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::Identifier;
    
    return Token(type, text, line_);
}

Token Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line_++;
        advance();
    }
    
    if (isAtEnd()) {
        return Token(TokenType::Error, "Unterminated string", line_);
    }
    
    advance(); // closing "
    
    // Trim quotes
    return Token(TokenType::String,
                 source_.substr(start_ + 1, current_ - start_ - 2),
                 line_);
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') {
            advance();
        } else if (c == '\n') {
            line_++;
            advance();
        } else if (c == '/' && peekNext() == '/') {
            // Line comment
            while (peek() != '\n' && !isAtEnd()) advance();
        } else {
            break;
        }
    }
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    return true;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

} // namespace volt
