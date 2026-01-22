#include "parser.h"
#include <sstream>

namespace volt {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ========== PROGRAM PARSING ==========

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> statements;
    
    while (!isAtEnd()) {
        try {
            statements.push_back(statement());
        } catch (...) {
            synchronize();
        }
    }
    
    return statements;
}

ExprPtr Parser::parseExpression() {
    try {
        return expression();
    } catch (...) {
        return nullptr;
    }
}

// ========== STATEMENT PARSING ==========

StmtPtr Parser::statement() {
    if (match(TokenType::Print)) return printStatement();
    if (match(TokenType::Let)) return letStatement();
    if (match(TokenType::Fn)) return fnStatement();
    if (match(TokenType::Return)) return returnStatement();
    if (match(TokenType::If)) return ifStatement();
    if (match(TokenType::While)) return whileStatement();
    if (match(TokenType::For)) return forStatement();
    if (match(TokenType::LeftBrace)) return blockStatement();
    
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after value");
    return std::make_unique<PrintStmt>(std::move(expr));
}

StmtPtr Parser::letStatement() {
    Token name = consume(TokenType::Identifier, "Expected variable name");
    
    ExprPtr initializer = nullptr;
    if (match(TokenType::Equal)) {
        initializer = expression();
    }
    
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    return std::make_unique<LetStmt>(std::string(name.lexeme), std::move(initializer));
}

StmtPtr Parser::fnStatement() {
    Token name = consume(TokenType::Identifier, "Expected function name");
    
    consume(TokenType::LeftParen, "Expected '(' after function name");
    
    // Parse parameters
    std::vector<std::string> parameters;
    if (!check(TokenType::RightParen)) {
        do {
            if (parameters.size() >= 255) {
                error("Can't have more than 255 parameters");
            }
            
            Token param = consume(TokenType::Identifier, "Expected parameter name");
            parameters.push_back(std::string(param.lexeme));
        } while (match(TokenType::Comma));
    }
    
    consume(TokenType::RightParen, "Expected ')' after parameters");
    consume(TokenType::LeftBrace, "Expected '{' before function body");
    
    // Parse body (statements until we hit closing brace)
    std::vector<StmtPtr> body;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        body.push_back(statement());
    }
    
    consume(TokenType::RightBrace, "Expected '}' after function body");
    
    return std::make_unique<FnStmt>(
        std::string(name.lexeme),
        std::move(parameters),
        std::move(body)
    );
}

StmtPtr Parser::returnStatement() {
    ExprPtr value = nullptr;
    
    // "return;" is valid (returns nil)
    if (!check(TokenType::Semicolon)) {
        value = expression();
    }
    
    consume(TokenType::Semicolon, "Expected ';' after return value");
    return std::make_unique<ReturnStmt>(std::move(value));
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'if'");
    ExprPtr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after if condition");
    
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    
    if (match(TokenType::Else)) {
        elseBranch = statement();
    }
    
    return std::make_unique<IfStmt>(std::move(condition), 
                                     std::move(thenBranch), 
                                     std::move(elseBranch));
}

StmtPtr Parser::whileStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'while'");
    ExprPtr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition");
    
    StmtPtr body = statement();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::forStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'for'");
    
    // Initializer
    StmtPtr initializer;
    if (match(TokenType::Semicolon)) {
        initializer = nullptr;
    } else if (match(TokenType::Let)) {
        initializer = letStatement();
    } else {
        initializer = expressionStatement();
    }
    
    // Condition
    ExprPtr condition = nullptr;
    if (!check(TokenType::Semicolon)) {
        condition = expression();
    }
    consume(TokenType::Semicolon, "Expected ';' after loop condition");
    
    // Increment
    ExprPtr increment = nullptr;
    if (!check(TokenType::RightParen)) {
        increment = expression();
    }
    consume(TokenType::RightParen, "Expected ')' after for clauses");
    
    StmtPtr body = statement();
    
    return std::make_unique<ForStmt>(std::move(initializer),
                                      std::move(condition),
                                      std::move(increment),
                                      std::move(body));
}

StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> statements;
    
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        statements.push_back(statement());
    }
    
    consume(TokenType::RightBrace, "Expected '}' after block");
    return std::make_unique<BlockStmt>(std::move(statements));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

// ========== EXPRESSION PARSING ==========

ExprPtr Parser::expression() {
    return assignment();
}

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

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    
    while (match(TokenType::Or)) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();
    
    while (match(TokenType::And)) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    
    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

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

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    
    while (match({TokenType::Plus, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::Bang, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return call();
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    
    // Handle chained calls: foo()()()
    while (match(TokenType::LeftParen)) {
        expr = finishCall(std::move(expr));
    }
    
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    
    if (!check(TokenType::RightParen)) {
        do {
            if (arguments.size() >= 255) {
                error("Can't have more than 255 arguments");
            }
            arguments.push_back(expression());
        } while (match(TokenType::Comma));
    }
    
    consume(TokenType::RightParen, "Expected ')' after arguments");
    
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

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
        consume(TokenType::RightParen, "Expected ')' after expression");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    
    error("Expected expression");
    return nullptr;
}

// ========== TOKEN HELPERS ==========

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

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(message);
    throw std::runtime_error(message);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

// ========== ERROR HANDLING ==========

void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << "[Line " << peek().line << "] Error: " << message;
    errors_.push_back(oss.str());
    hadError_ = true;
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
            case TokenType::Print:
                return;
            default:
                break;
        }
        
        advance();
    }
}

} // namespace volt
