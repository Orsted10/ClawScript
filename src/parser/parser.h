#pragma once
#include "ast.h"
#include "stmt.h"
#include "token.h"
#include <vector>
#include <string>

namespace volt {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    // Parse program (list of statements)
    std::vector<StmtPtr> parseProgram();
    
    // Parse single expression (for REPL/testing)
    ExprPtr parseExpression();
    
    // Check for errors
    bool hadError() const { return hadError_; }
    const std::vector<std::string>& getErrors() const { return errors_; }

private:
    // Statement parsing
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr letStatement();
    StmtPtr fnStatement();
    StmtPtr returnStatement();
    StmtPtr breakStatement();
    StmtPtr continueStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr runUntilStatement();
    StmtPtr forStatement();
    StmtPtr blockStatement();
    StmtPtr expressionStatement();
    
    // Expression parsing (by precedence level)
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr ternary();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr postfix();
    ExprPtr call();
    ExprPtr primary();

    // Array parsing - Added!
    ExprPtr arrayLiteral();
    ExprPtr finishIndexOrMember(ExprPtr object);

    // Hash map parsing - Added!
    ExprPtr hashMapLiteral();

    // Helper to finish call expressions
    ExprPtr finishCall(ExprPtr callee);
    
    // Token manipulation
    Token advance();
    Token peek() const;
    Token previous() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
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
