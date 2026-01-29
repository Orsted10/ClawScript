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
    if (match(TokenType::Break)) return breakStatement();
    if (match(TokenType::Continue)) return continueStatement();
    if (match(TokenType::If)) return ifStatement();
    if (match(TokenType::While)) return whileStatement();
    if (match(TokenType::Run)) return runUntilStatement();
    if (match(TokenType::For)) return forStatement();
    if (match(TokenType::LeftBrace)) return blockStatement();
    
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    Token keyword = previous();
    ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after value");
    return std::make_unique<PrintStmt>(keyword, std::move(expr));
}

StmtPtr Parser::letStatement() {
    Token keyword = previous();
    Token name = consume(TokenType::Identifier, "Expected variable name");
    
    ExprPtr initializer = nullptr;
    if (match(TokenType::Equal)) {
        initializer = expression();
    }
    
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    return std::make_unique<LetStmt>(name, std::move(initializer));
}

StmtPtr Parser::fnStatement() {
    Token keyword = previous();
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
        name,
        std::move(parameters),
        std::move(body)
    );
}

StmtPtr Parser::returnStatement() {
    Token keyword = previous();
    ExprPtr value = nullptr;
    
    // "return;" is valid (returns nil)
    if (!check(TokenType::Semicolon)) {
        value = expression();
    }
    
    consume(TokenType::Semicolon, "Expected ';' after return value");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

StmtPtr Parser::breakStatement() {
    Token keyword = previous();
    consume(TokenType::Semicolon, "Expected ';' after 'break'");
    return std::make_unique<BreakStmt>(keyword);
}

StmtPtr Parser::continueStatement() {
    Token keyword = previous();
    consume(TokenType::Semicolon, "Expected ';' after 'continue'");
    return std::make_unique<ContinueStmt>(keyword);
}

StmtPtr Parser::ifStatement() {
    Token keyword = previous();
    consume(TokenType::LeftParen, "Expected '(' after 'if'");
    ExprPtr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after if condition");
    
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    
    if (match(TokenType::Else)) {
        elseBranch = statement();
    }
    
    return std::make_unique<IfStmt>(keyword, std::move(condition), 
                                     std::move(thenBranch), 
                                     std::move(elseBranch));
}

StmtPtr Parser::whileStatement() {
    Token keyword = previous();
    consume(TokenType::LeftParen, "Expected '(' after 'while'");
    ExprPtr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition");
    
    StmtPtr body = statement();
    
    return std::make_unique<WhileStmt>(keyword, std::move(condition), std::move(body));
}

StmtPtr Parser::runUntilStatement() {
    Token keyword = previous(); // 'run' token
    
    // Parse body (must be a statement)
    StmtPtr body = statement();
    
    // Expect 'until' keyword
    consume(TokenType::Until, "Expected 'until' after run body");
    
    // Parse condition
    consume(TokenType::LeftParen, "Expected '(' after 'until'");
    ExprPtr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition");
    consume(TokenType::Semicolon, "Expected ';' after run-until statement");
    
    return std::make_unique<RunUntilStmt>(keyword, std::move(body), std::move(condition));
}

StmtPtr Parser::forStatement() {
    Token keyword = previous();
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
    
    return std::make_unique<ForStmt>(keyword, std::move(initializer),
                                      std::move(condition),
                                      std::move(increment),
                                      std::move(body));
}

StmtPtr Parser::blockStatement() {
    Token brace = previous();
    std::vector<StmtPtr> statements;
    
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        statements.push_back(statement());
    }
    
    consume(TokenType::RightBrace, "Expected '}' after block");
    return std::make_unique<BlockStmt>(brace, std::move(statements));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    Token tok = expr->token; // Use the expression's own token
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(tok, std::move(expr));
}

// ========== EXPRESSION PARSING ==========

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = ternary();
    
    // Handle regular assignment: =
    if (match(TokenType::Equal)) {
        Token equals = previous();
        ExprPtr value = assignment();
        
        // Variable assignment: x = 10
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->token, std::move(value));
        }
        
        // Array index assignment: arr[0] = 42  // Added!
        if (auto* index = dynamic_cast<IndexExpr*>(expr.get())) {
            return std::make_unique<IndexAssignExpr>(
                index->token,
                std::move(index->object),
                std::move(index->index),
                std::move(value)
            );
        }
        
        error("Invalid assignment target");
    }
    
    // Compound assignment: +=, -=, *=, /=
    if (match({TokenType::PlusEqual, TokenType::MinusEqual, 
               TokenType::StarEqual, TokenType::SlashEqual})) {
        Token op = previous();
        ExprPtr value = assignment();
        
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<CompoundAssignExpr>(var->token, op, std::move(value));
        }
        
        error("Invalid compound assignment target");
    }
    
    return expr;
}

ExprPtr Parser::ternary() {
    ExprPtr expr = logicalOr();
    
    if (match(TokenType::Question)) {
        Token quest = previous();
        ExprPtr thenBranch = expression();  // Allow nested ternary
        consume(TokenType::Colon, "Expected ':' in ternary expression");
        ExprPtr elseBranch = ternary();  // Right-associative
        expr = std::make_unique<TernaryExpr>(
            quest, std::move(expr), std::move(thenBranch), std::move(elseBranch));
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
    // Prefix unary: !, -
    if (match({TokenType::Bang, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    // Prefix increment/decrement: ++x, --x
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        // Next must be an identifier
        if (match(TokenType::Identifier)) {
            Token nameTok = previous();
            return std::make_unique<UpdateExpr>(nameTok, op, true);
        }
        error("Expected identifier after '" + std::string(op.lexeme) + "'");
        return nullptr;
    }
    
    return postfix();
}

ExprPtr Parser::postfix() {
    ExprPtr expr = call();
    
    // Postfix increment/decrement: x++, x--
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<UpdateExpr>(var->token, op, false);
        }
        error("Invalid postfix operand");
    }
    
    return expr;
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    
    while (true) {
        if (match(TokenType::LeftParen)) {
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::LeftBracket)) {  // NEW: Array indexing
            expr = finishIndexOrMember(std::move(expr));
        } else if (match(TokenType::Dot)) {  // NEW: Member access
            Token name = consume(TokenType::Identifier, "Expected property name after '.'");
            expr = std::make_unique<MemberExpr>(name, std::move(expr), std::string(name.lexeme));
        } else {
            break;
        }
    }
    
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    Token paren = previous(); // This is the '('
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
    
    return std::make_unique<CallExpr>(paren, std::move(callee), std::move(arguments));
}

ExprPtr Parser::arrayLiteral() {
    Token bracket = previous(); // This is the '['
    std::vector<ExprPtr> elements;
    
    // Empty array: []
    if (check(TokenType::RightBracket)) {
        advance();
        return std::make_unique<ArrayExpr>(bracket, std::move(elements));
    }
    
    // Parse elements: [1, 2, 3]
    do {
        // Allow trailing comma
        if (check(TokenType::RightBracket)) break;
        
        elements.push_back(expression());
    } while (match(TokenType::Comma));
    
    consume(TokenType::RightBracket, "Expected ']' after array elements");
    
    return std::make_unique<ArrayExpr>(bracket, std::move(elements));
}

// Hash map literal: {"key": "value", "age": 25}  // Added!
ExprPtr Parser::hashMapLiteral() {
    Token brace = previous(); // This is the '{'
    std::vector<std::pair<ExprPtr, ExprPtr>> keyValuePairs;
    
    // Empty hash map: {}
    if (check(TokenType::RightBrace)) {
        advance();
        return std::make_unique<HashMapExpr>(brace, std::move(keyValuePairs));
    }
    
    // Parse key-value pairs: {"key": value, "another": value2}
    do {
        // Allow trailing comma
        if (check(TokenType::RightBrace)) break;
        
        // Parse key (must be string, number, or identifier that resolves to string)
        ExprPtr key;
        if (match(TokenType::String)) {
            Token tok = previous();
            key = std::make_unique<LiteralExpr>(tok, tok.stringValue);
        } else if (match(TokenType::Number)) {
            Token tok = previous();
            double value = std::stod(std::string(tok.lexeme));
            key = std::make_unique<LiteralExpr>(tok, value);
        } else if (match(TokenType::True)) {
            Token tok = previous();
            key = std::make_unique<LiteralExpr>(tok, true);
        } else if (match(TokenType::False)) {
            Token tok = previous();
            key = std::make_unique<LiteralExpr>(tok, false);
        } else if (match(TokenType::Nil)) {
            Token tok = previous();
            key = LiteralExpr::nil(tok);
        } else {
            Token tok = consume(TokenType::Identifier, "Expected string, number, or identifier as key");
            key = std::make_unique<LiteralExpr>(tok, std::string(tok.lexeme)); // Treat identifier as string key
        }
        
        // Expect colon separator
        consume(TokenType::Colon, "Expected ':' after key");
        
        // Parse value
        ExprPtr value = expression();
        
        // Add key-value pair
        keyValuePairs.emplace_back(std::move(key), std::move(value));
    } while (match(TokenType::Comma));
    
    consume(TokenType::RightBrace, "Expected '}' after hash map elements");
    
    return std::make_unique<HashMapExpr>(brace, std::move(keyValuePairs));
}


ExprPtr Parser::finishIndexOrMember(ExprPtr object) {
    Token bracket = previous(); // This is the '['
    ExprPtr index = expression();
    consume(TokenType::RightBracket, "Expected ']' after array index");
    
    // Create index expression
    return std::make_unique<IndexExpr>(bracket, std::move(object), std::move(index));
}

ExprPtr Parser::primary() {
    if (match(TokenType::Number)) {
        Token tok = previous();
        double value = std::stod(std::string(tok.lexeme));
        return std::make_unique<LiteralExpr>(tok, value);
    }
    
    if (match(TokenType::String)) {
        Token tok = previous();
        return std::make_unique<LiteralExpr>(tok, tok.stringValue);
    }
    
    if (match(TokenType::True)) {
        return std::make_unique<LiteralExpr>(previous(), true);
    }
    
    if (match(TokenType::False)) {
        return std::make_unique<LiteralExpr>(previous(), false);
    }
    
    if (match(TokenType::Nil)) {
        return LiteralExpr::nil(previous());
    }
    
    if (match(TokenType::Identifier)) {
        Token tok = previous();
        return std::make_unique<VariableExpr>(tok, std::string(tok.lexeme));
    }
    
    if (match(TokenType::LeftParen)) {
        Token tok = previous();
        ExprPtr expr = expression();
        consume(TokenType::RightParen, "Expected ')' after expression");
        return std::make_unique<GroupingExpr>(tok, std::move(expr));
    }
    
    // Array literal: [1, 2, 3]  // Added!
    if (match(TokenType::LeftBracket)) {
        return arrayLiteral();
    }

    // Hash map literal: {"key": "value", "age": 25}  // Added!
    if (match(TokenType::LeftBrace)) {
        return hashMapLiteral();
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
    Token tok = peek();
    std::ostringstream oss;
    oss << "[Line " << tok.line << ", Col " << tok.column << "] Error";
    if (tok.type == TokenType::Eof) {
        oss << " at end";
    } else {
        oss << " at '" << tok.lexeme << "'";
    }
    oss << ": " << message;
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
