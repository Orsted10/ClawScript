#include "token.h"

namespace volt {

const char* tokenName(TokenType type) {
    switch (type) {
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Let: return "Let";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::For: return "For";
        case TokenType::Fn: return "Fn";
        case TokenType::Return: return "Return";
        case TokenType::Print: return "Print";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Nil: return "Nil";
        case TokenType::Break: return "Break";
        case TokenType::Continue: return "Continue";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Percent: return "Percent";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::Bang: return "Bang";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::Less: return "Less";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::Greater: return "Greater";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::And: return "And";
        case TokenType::Or: return "Or";
        case TokenType::PlusEqual: return "PlusEqual";
        case TokenType::MinusEqual: return "MinusEqual";
        case TokenType::StarEqual: return "StarEqual";
        case TokenType::SlashEqual: return "SlashEqual";
        case TokenType::PlusPlus: return "PlusPlus";
        case TokenType::MinusMinus: return "MinusMinus";
        case TokenType::Question: return "Question";
        case TokenType::Colon: return "Colon";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";   // NEW!
        case TokenType::RightBracket: return "RightBracket"; // NEW!
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";  // NEW!
        case TokenType::Eof: return "Eof";
        case TokenType::Error: return "Error";
        default: return "Unknown";
    }
}

} // namespace volt
