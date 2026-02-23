#include "parser.h"
#include <sstream>
#include <iostream>

namespace claw {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ========== PROGRAM PARSING ==========

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> statements;
    
    while (!isAtEnd()) {
        try {
            StmtPtr stmt = statement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
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
    if (match(TokenType::Try)) return tryStatement();
    if (match(TokenType::Throw)) return throwStatement();
    if (match(TokenType::Import)) return importStatement();
    if (match(TokenType::Class)) return classStatement();
    if (match(TokenType::Switch)) return switchStatement();
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
        StmtPtr stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
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

StmtPtr Parser::tryStatement() {
    Token keyword = previous(); // 'try' token
    
    // Parse try body
    StmtPtr tryBody = statement();
    
    if (!tryBody) {
        return nullptr;
    }
    
    // Expect 'catch' keyword
    consume(TokenType::Catch, "Expected 'catch' after try block");
    
    // Parse exception variable
    consume(TokenType::LeftParen, "Expected '(' after 'catch'");
    Token exceptionVar = consume(TokenType::Identifier, "Expected exception variable name");
    consume(TokenType::RightParen, "Expected ')' after exception variable");
    
    // Parse catch body
    StmtPtr catchBody = statement();
    
    if (!catchBody) {
        return nullptr;
    }
    
    return std::make_unique<TryStmt>(keyword, std::move(tryBody), 
                                     std::string(exceptionVar.lexeme), 
                                     std::move(catchBody));
}

StmtPtr Parser::throwStatement() {
    Token keyword = previous();
    ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after throw expression");
    return std::make_unique<ThrowStmt>(keyword, std::move(expr));
}

StmtPtr Parser::importStatement() {
    Token keyword = previous();
    
    std::vector<std::string> imports;
    consume(TokenType::LeftBrace, "Expected '{' after 'import'");
    
    if (!check(TokenType::RightBrace)) {
        do {
            Token importName = consume(TokenType::Identifier, "Expected import name");
            imports.push_back(std::string(importName.lexeme));
        } while (match(TokenType::Comma));
    }
    
    consume(TokenType::RightBrace, "Expected '}' after import list");
    consume(TokenType::From, "Expected 'from' after import list");
    
    Token modulePathTok = consume(TokenType::String, "Expected module path string");
    std::string modulePath = std::string(modulePathTok.stringValue);
    
    consume(TokenType::Semicolon, "Expected ';' after import statement");
    
    return std::make_unique<ImportStmt>(keyword, std::move(imports), std::move(modulePath));
}

StmtPtr Parser::classStatement() {
    Token keyword = previous();
    Token name = consume(TokenType::Identifier, "Expected class name");

    ExprPtr superclass = nullptr;
    if (match(TokenType::Less)) {
        consume(TokenType::Identifier, "Expected superclass name");
        superclass = std::make_unique<VariableExpr>(previous(), std::string(previous().lexeme));
    }

    consume(TokenType::LeftBrace, "Expected '{' before class body");

    std::vector<std::unique_ptr<FnStmt>> methods;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        consume(TokenType::Fn, "Expected method declaration in class body");
        StmtPtr method = fnStatement();
        // Cast StmtPtr to unique_ptr<FnStmt>
        auto* fnStmt = dynamic_cast<FnStmt*>(method.get());
        if (fnStmt) {
            method.release(); // Release ownership from StmtPtr
            methods.push_back(std::unique_ptr<FnStmt>(fnStmt));
        } else {
            error("Expected method declaration");
        }
    }

    consume(TokenType::RightBrace, "Expected '}' after class body");

    return std::make_unique<ClassStmt>(name, std::move(superclass), std::move(methods));
}

StmtPtr Parser::switchStatement() {
    Token keyword = previous();
    consume(TokenType::LeftParen, "Expected '(' after 'switch'");
    ExprPtr expr = expression();
    consume(TokenType::RightParen, "Expected ')' after switch expression");
    consume(TokenType::LeftBrace, "Expected '{' before switch body");
    
    std::vector<SwitchStmt::Case> cases;
    
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        if (match(TokenType::Case)) {
            ExprPtr matchExpr = expression();
            consume(TokenType::Colon, "Expected ':' after case expression");
            std::vector<StmtPtr> body;
            while (!check(TokenType::RightBrace) && !check(TokenType::Case) && !check(TokenType::Default) && !isAtEnd()) {
                body.push_back(statement());
            }
            cases.push_back(SwitchStmt::Case{false, std::move(matchExpr), std::move(body)});
        } else if (match(TokenType::Default)) {
            consume(TokenType::Colon, "Expected ':' after default");
            std::vector<StmtPtr> body;
            while (!check(TokenType::RightBrace) && !check(TokenType::Case) && !isAtEnd()) {
                body.push_back(statement());
            }
            cases.push_back(SwitchStmt::Case{true, nullptr, std::move(body)});
        } else {
            error("Expected 'case' or 'default' in switch body");
            break;
        }
    }
    
    consume(TokenType::RightBrace, "Expected '}' after switch body");
    return std::make_unique<SwitchStmt>(keyword, std::move(expr), std::move(cases));
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
        
        // Property assignment: obj.property = value  // Added!
        if (auto* member = dynamic_cast<MemberExpr*>(expr.get())) {
            return std::make_unique<SetExpr>(
                member->token,
                std::move(member->object),
                member->member,
                std::move(value)
            );
        }
        
        error("Invalid assignment target");
    }
    
    // Compound assignment: +=, -=, *=, /= and bitwise/shift variants
    if (match({TokenType::PlusEqual, TokenType::MinusEqual, 
               TokenType::StarEqual, TokenType::SlashEqual,
               TokenType::BitAndEqual, TokenType::BitOrEqual, TokenType::BitXorEqual,
               TokenType::ShiftLeftEqual, TokenType::ShiftRightEqual})) {
        Token op = previous();
        ExprPtr value = assignment();
        
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<CompoundAssignExpr>(var->token, op, std::move(value));
        }
        if (auto* member = dynamic_cast<MemberExpr*>(expr.get())) {
            auto raw = static_cast<MemberExpr*>(expr.release());
            auto result = std::make_unique<CompoundMemberAssignExpr>(
                raw->token, std::move(raw->object), raw->member, op, std::move(value));
            delete raw;
            return result;
        }
        if (auto* index = dynamic_cast<IndexExpr*>(expr.get())) {
            auto raw = static_cast<IndexExpr*>(expr.release());
            auto result = std::make_unique<CompoundIndexAssignExpr>(
                raw->token, std::move(raw->object), std::move(raw->index), op, std::move(value));
            delete raw;
            return result;
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
    ExprPtr expr = bitwiseOr();
    
    while (match(TokenType::And)) {
        Token op = previous();
        ExprPtr right = bitwiseOr();
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
    ExprPtr expr = shift();
    
    while (match({TokenType::Greater, TokenType::GreaterEqual,
                  TokenType::Less, TokenType::LessEqual})) {
        Token op = previous();
        ExprPtr right = shift();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// Bitwise OR: |
ExprPtr Parser::bitwiseOr() {
    ExprPtr expr = bitwiseXor();
    
    while (match(TokenType::BitOr)) {
        Token op = previous();
        ExprPtr right = bitwiseXor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// Bitwise XOR: ^
ExprPtr Parser::bitwiseXor() {
    ExprPtr expr = bitwiseAnd();
    
    while (match(TokenType::BitXor)) {
        Token op = previous();
        ExprPtr right = bitwiseAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// Bitwise AND: &
ExprPtr Parser::bitwiseAnd() {
    ExprPtr expr = equality();
    
    while (match(TokenType::BitAnd)) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

// Shift operators: <<, >>
ExprPtr Parser::shift() {
    ExprPtr expr = term();
    
    while (match({TokenType::ShiftLeft, TokenType::ShiftRight})) {
        Token op = previous();
        ExprPtr right = unary();
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
    if (match({TokenType::Bang, TokenType::Minus, TokenType::BitNot})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        ExprPtr target = postfix();
        if (!target) {
            error("Invalid prefix operand");
            return nullptr;
        }
        if (auto* var = dynamic_cast<VariableExpr*>(target.get())) {
            return std::make_unique<UpdateExpr>(var->token, op, true);
        }
        if (auto* member = dynamic_cast<MemberExpr*>(target.get())) {
            // Steal underlying pointers to avoid double evaluation
            auto raw = static_cast<MemberExpr*>(target.release());
            auto result = std::make_unique<UpdateMemberExpr>(raw->token, std::move(raw->object), raw->member, op, true);
            delete raw;
            return result;
        }
        if (auto* index = dynamic_cast<IndexExpr*>(target.get())) {
            auto raw = static_cast<IndexExpr*>(target.release());
            auto result = std::make_unique<UpdateIndexExpr>(raw->token, std::move(raw->object), std::move(raw->index), op, true);
            delete raw;
            return result;
        }
        error("Invalid prefix operand");
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
        if (auto* member = dynamic_cast<MemberExpr*>(expr.get())) {
            auto raw = static_cast<MemberExpr*>(expr.release());
            auto result = std::make_unique<UpdateMemberExpr>(raw->token, std::move(raw->object), raw->member, op, false);
            delete raw;
            return result;
        }
        if (auto* index = dynamic_cast<IndexExpr*>(expr.get())) {
            auto raw = static_cast<IndexExpr*>(expr.release());
            auto result = std::make_unique<UpdateIndexExpr>(raw->token, std::move(raw->object), std::move(raw->index), op, false);
            delete raw;
            return result;
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

// Function expression parsing - Added!
ExprPtr Parser::functionExpression() {
    Token keyword = previous(); // This is the 'fun' token
    
    consume(TokenType::LeftParen, "Expected '(' after 'fun'");
    
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
        StmtPtr stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }
    
    consume(TokenType::RightBrace, "Expected '}' after function body");
    
    // Create function expression (similar to FnStmt but as an expression)
    return std::make_unique<FunctionExpr>(keyword, std::move(parameters), std::move(body));
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
    
    if (match(TokenType::This)) {
        return std::make_unique<ThisExpr>(previous());
    }

    if (match(TokenType::Super)) {
        Token keyword = previous();
        consume(TokenType::Dot, "Expected '.' after 'super'");
        Token method = consume(TokenType::Identifier, "Expected superclass method name");
        return std::make_unique<SuperExpr>(keyword, std::string(method.lexeme));
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

    // Function expression: fun(params) { body }  // Added!
    if (match(TokenType::Fn)) {
        return functionExpression();
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
    oss << "E1001: Syntax Error [Line " << tok.line << ", Col " << tok.column << "]";
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

} // namespace claw
