#include "token.h"

namespace volt {

const char* tokenName(TokenType type) {
    switch (type) {
        case TokenType::Number: return "NUMBER";
        case TokenType::String: return "STRING";
        case TokenType::Identifier: return "IDENTIFIER";
        
        case TokenType::Let: return "LET";
        case TokenType::If: return "IF";
        case TokenType::Else: return "ELSE";
        case TokenType::While: return "WHILE";
        case TokenType::For: return "FOR";
        case TokenType::Run: return "RUN";
        case TokenType::Until: return "UNTIL";
        case TokenType::Fn: return "FN";
        case TokenType::Return: return "RETURN";
        case TokenType::True: return "TRUE";
        case TokenType::False: return "FALSE";
        case TokenType::Nil: return "NIL";
        case TokenType::Print: return "PRINT";
        
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Equal: return "EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::Bang: return "BANG";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::And: return "AND";
        case TokenType::Or: return "OR";
        
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::Comma: return "COMMA";
        
        case TokenType::Eof: return "EOF";
        case TokenType::Error: return "ERROR";
        
        default: return "UNKNOWN";
    }
}

} // namespace volt
