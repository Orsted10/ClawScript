#include "formatter.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include <sstream>
#include <string>

namespace claw::fmt {

static bool isBinaryOp(TokenType t) {
    switch (t) {
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::Star:
        case TokenType::Slash:
        case TokenType::Percent:
        case TokenType::Equal:
        case TokenType::EqualEqual:
        case TokenType::BangEqual:
        case TokenType::Less:
        case TokenType::LessEqual:
        case TokenType::Greater:
        case TokenType::GreaterEqual:
        case TokenType::PlusEqual:
        case TokenType::MinusEqual:
        case TokenType::StarEqual:
        case TokenType::SlashEqual:
        case TokenType::And:
        case TokenType::Or:
            return true;
        default: return false;
    }
}

std::string formatTokens(const std::vector<claw::Token>& tokens, int indentSize) {
    std::ostringstream out;
    int indent = 0;
    bool newLine = true;
    auto writeIndent = [&]() {
        for (int i = 0; i < indent * indentSize; ++i) out << ' ';
    };
    TokenType prevType = TokenType::Eof;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        if (t.type == TokenType::Eof) break;
        if (t.type == TokenType::RightBrace) {
            if (!newLine) { out << "\n"; newLine = true; }
            indent = std::max(0, indent - 1);
            writeIndent();
            out << "}";
            newLine = false;
            prevType = t.type;
            continue;
        }
        if (newLine) { writeIndent(); newLine = false; }
        if (t.type == TokenType::LeftBrace) {
            out << " {";
            newLine = true;
            out << "\n";
            indent++;
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::Semicolon) {
            out << ";";
            out << "\n";
            newLine = true;
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::RightParen) {
            out << ")";
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::RightBracket) {
            out << "]";
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::LeftParen) {
            out << "(";
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::LeftBracket) {
            out << "[";
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::Comma) {
            out << ", ";
            prevType = t.type;
            continue;
        }
        if (t.type == TokenType::Colon) {
            out << ": ";
            prevType = t.type;
            continue;
        }
        if (isBinaryOp(t.type)) {
            out << " " << std::string(t.lexeme) << " ";
        } else {
            if (prevType == TokenType::Identifier ||
                prevType == TokenType::Number ||
                prevType == TokenType::String) {
                if (t.type == TokenType::Identifier ||
                    t.type == TokenType::Number ||
                    t.type == TokenType::String) {
                    out << " ";
                }
            }
            out << std::string(t.lexeme);
        }
        prevType = t.type;
    }
    return out.str();
}

} // namespace claw::fmt
