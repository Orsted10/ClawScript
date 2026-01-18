#pragma once
#include "ast.h"
#include "token.h"
#include <vector>
#include <string>

namespace volt {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    // Parse an expression
    ExprPtr parse();
    
    // Check for errors
    bool hadError() const { return hadError_; }
    const std::vector<std::string>& getErrors() const { return errors_; }

private:
    // Expression parsing (by precedence level)
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr primary();
    
    // Helper to finish call expressions
    ExprPtr finishCall(ExprPtr callee);
    
    // Token manipulation
    Token advance();
    Token peek() const;
    Token previous() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    bool isAtEnd() const;
    
    // Error handling
    void error(const std::string& message);
    void synchronize();
    
    std::vector<Token> tokens_;
    size_t current_ = 0;
    bool hadError_ = false;
    std::vector<std::string> errors_;
};

} // namespace volt
