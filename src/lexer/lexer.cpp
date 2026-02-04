#include "lexer.h"
#include <iostream>

namespace volt {

const std::unordered_map<std::string_view, TokenType> Lexer::keywords_ = {
    {"let", TokenType::Let},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"run", TokenType::Run},
    {"until", TokenType::Until},
    {"fn", TokenType::Fn},
    {"return", TokenType::Return},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"nil", TokenType::Nil},
    {"print", TokenType::Print},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"try", TokenType::Try},
    {"catch", TokenType::Catch},
    {"fun", TokenType::Fn},  // Alias for fn
};

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;
        
        start_ = current_;
        startColumn_ = column_;
        tokens.push_back(scanToken());
    }
    
    tokens.push_back(Token(TokenType::Eof, "", line_, column_));
    return tokens;
}

Token Lexer::scanToken() {
    start_ = current_;
    startColumn_ = column_;
    char c = advance();
    
    if (isDigit(c)) return number();
    if (isAlpha(c)) return identifier();
    if (c == '"') return string();
    
    switch (c) {
        case '(': return Token(TokenType::LeftParen, "(", line_, startColumn_);
        case ')': return Token(TokenType::RightParen, ")", line_, startColumn_);
        case '{': return Token(TokenType::LeftBrace, "{", line_, startColumn_);
        case '}': return Token(TokenType::RightBrace, "}", line_, startColumn_);
        case ';': return Token(TokenType::Semicolon, ";", line_, startColumn_);
        case ',': return Token(TokenType::Comma, ",", line_, startColumn_);
        case '?': return Token(TokenType::Question, "?", line_, startColumn_);
        case ':': return Token(TokenType::Colon, ":", line_, startColumn_);
        case '%': return Token(TokenType::Percent, "%", line_, startColumn_);
        case '[': return Token(TokenType::LeftBracket, "[", line_, startColumn_);
        case ']': return Token(TokenType::RightBracket, "]", line_, startColumn_);
        case '.': return Token(TokenType::Dot, ".", line_, startColumn_);

        
        case '+':
            if (match('+')) return Token(TokenType::PlusPlus, "++", line_, startColumn_);
            if (match('=')) return Token(TokenType::PlusEqual, "+=", line_, startColumn_);
            return Token(TokenType::Plus, "+", line_, startColumn_);
        
        case '-':
            if (match('-')) return Token(TokenType::MinusMinus, "--", line_, startColumn_);
            if (match('=')) return Token(TokenType::MinusEqual, "-=", line_, startColumn_);
            return Token(TokenType::Minus, "-", line_, startColumn_);
        
        case '*':
            if (match('=')) return Token(TokenType::StarEqual, "*=", line_, startColumn_);
            return Token(TokenType::Star, "*", line_, startColumn_);
        
        case '/':
            if (match('=')) return Token(TokenType::SlashEqual, "/=", line_, startColumn_);
            return Token(TokenType::Slash, "/", line_, startColumn_);
        
        case '=':
            if (match('=')) {
                return Token(TokenType::EqualEqual, "==", line_, startColumn_);
            }
            return Token(TokenType::Equal, "=", line_, startColumn_);
        
        case '!':
            if (match('=')) {
                return Token(TokenType::BangEqual, "!=", line_, startColumn_);
            }
            return Token(TokenType::Bang, "!", line_, startColumn_);
        
        case '<':
            if (match('=')) {
                return Token(TokenType::LessEqual, "<=", line_, startColumn_);
            }
            return Token(TokenType::Less, "<", line_, startColumn_);
        
        case '>':
            if (match('=')) {
                return Token(TokenType::GreaterEqual, ">=", line_, startColumn_);
            }
            return Token(TokenType::Greater, ">", line_, startColumn_);
        
        case '&':
            if (match('&')) {
                return Token(TokenType::And, "&&", line_, startColumn_);
            }
            break;
        
        case '|':
            if (match('|')) {
                return Token(TokenType::Or, "||", line_, startColumn_);
            }
            break;
    }
    
    return Token(TokenType::Error, source_.substr(start_, 1), line_, startColumn_);
}

Token Lexer::number() {
    while (isDigit(peek())) advance();
    
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }
    
    return Token(TokenType::Number, 
                 source_.substr(start_, current_ - start_), 
                 line_, startColumn_);
}

Token Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();
    
    std::string_view text = source_.substr(start_, current_ - start_);
    
    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::Identifier;
    
    return Token(type, text, line_, startColumn_);
}

Token Lexer::string() {
    int stringStartColumn = startColumn_;
    std::string processed;
    
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line_++;
            column_ = 0;  // Will be incremented by advance()
        }
        
        // Handle escape sequences
        if (peek() == '\\' && !isAtEnd()) {
            advance();  // consume backslash
            if (!isAtEnd()) {
                char escaped = advance();
                switch (escaped) {
                    case 'n': processed += '\n'; break;
                    case 't': processed += '\t'; break;
                    case 'r': processed += '\r'; break;
                    case '\\': processed += '\\'; break;
                    case '"': processed += '"'; break;
                    case '0': processed += '\0'; break;
                    default:
                        // Unknown escape, keep as-is
                        processed += '\\';
                        processed += escaped;
                        break;
                }
            }
        } else {
            processed += advance();
        }
    }
    
    if (isAtEnd()) {
        return Token(TokenType::Error, "Unterminated string", line_, stringStartColumn);
    }
    
    advance(); // closing "
    
    // Return token with both raw lexeme and processed value
    std::string_view rawLexeme = source_.substr(start_, current_ - start_);
    return Token(TokenType::String, rawLexeme, line_, stringStartColumn, std::move(processed));
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') {
            advance();
        } else if (c == '\n') {
            line_++;
            column_ = 0;  // Will be incremented by advance()
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
    column_++;
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
    column_++;
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
