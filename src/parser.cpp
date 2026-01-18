#include "parser.h"
#include <sstream>

namespace volt {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

ExprPtr Parser::parse() {
    try {
        return expression();
    } catch (...) {
        return nullptr;
    }
}

// expression → assignment
ExprPtr Parser::expression() {
    return assignment();
}

// assignment → IDENTIFIER "=" assignment | logicalOr
ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();
    
    if (match(TokenType::Equal)) {
        Token equals = previous();
        ExprPtr value = assignment();
        
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        }
        
        error("Invalid assignment target");
    }
    
    return expr;
}

// logicalOr → logicalAnd ( "||" logicalAnd )*
ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    
    while (match(TokenType::Or)) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// logicalAnd → equality ( "&&" equality )*
ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();
    
    while (match(TokenType::And)) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// equality → comparison ( ( "==" | "!=" ) comparison )*
ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    
    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// comparison → term ( ( ">" | ">=" | "<" | "<=" ) term )*
ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    
    while (match({TokenType::Greater, TokenType::GreaterEqual,
                  TokenType::Less, TokenType::LessEqual})) {
        Token op = previous();
        ExprPtr right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// term → factor ( ( "+" | "-" ) factor )*
ExprPtr Parser::term() {
    ExprPtr expr = factor();
    
    while (match({TokenType::Plus, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// factor → unary ( ( "*" | "/" | "%" ) unary )*
ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// unary → ( "!" | "-" ) unary | call
ExprPtr Parser::unary() {
    if (match({TokenType::Bang, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return call();
}

// call → primary ( "(" arguments? ")" )*
ExprPtr Parser::call() {
    ExprPtr expr = primary();
    
    while (match(TokenType::LeftParen)) {
        expr = finishCall(std::move(expr));
    }
    
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    
    if (!check(TokenType::RightParen)) {
        do {
            arguments.push_back(expression());
        } while (match(TokenType::Comma));
    }
    
    if (!match(TokenType::RightParen)) {
        error("Expected ')' after arguments");
    }
    
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

// primary → NUMBER | STRING | "true" | "false" | "nil" | IDENTIFIER | "(" expression ")"
ExprPtr Parser::primary() {
    if (match(TokenType::Number)) {
        double value = std::stod(std::string(previous().lexeme));
        return std::make_unique<LiteralExpr>(value);
    }
    
    if (match(TokenType::String)) {
        return std::make_unique<LiteralExpr>(std::string(previous().lexeme));
    }
    
    if (match(TokenType::True)) {
        return std::make_unique<LiteralExpr>(true);
    }
    
    if (match(TokenType::False)) {
        return std::make_unique<LiteralExpr>(false);
    }
    
    if (match(TokenType::Nil)) {
        return LiteralExpr::nil();
    }
    
    if (match(TokenType::Identifier)) {
        return std::make_unique<VariableExpr>(std::string(previous().lexeme));
    }
    
    if (match(TokenType::LeftParen)) {
        ExprPtr expr = expression();
        if (!match(TokenType::RightParen)) {
            error("Expected ')' after expression");
        }
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    
    error("Expected expression");
    return nullptr;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

Token Parser::peek() const {
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << "[Line " << peek().line << "] Error: " << message;
    errors_.push_back(oss.str());
    hadError_ = true;
    throw std::runtime_error(message);
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        
        switch (peek().type) {
            case TokenType::If:
            case TokenType::While:
            case TokenType::For:
            case TokenType::Fn:
            case TokenType::Return:
            case TokenType::Let:
                return;
            default:
                break;
        }
        
        advance();
    }
}

} // namespace volt
