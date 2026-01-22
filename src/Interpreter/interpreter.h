#pragma once
#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

namespace volt {

/**
 * Interpreter - Executes statements and evaluates expressions
 */
class Interpreter {
public:
    Interpreter();
    
    // Execute statements
    void execute(Stmt* stmt);
    void execute(const std::vector<StmtPtr>& statements);
    
    // Evaluate expressions
    Value evaluate(Expr* expr);
    
    // Get current environment
    std::shared_ptr<Environment> getEnvironment() { return environment_; }
    
    // Reset interpreter
    void reset();

private:
    // Statement execution
    void executeExprStmt(ExprStmt* stmt);
    void executePrintStmt(PrintStmt* stmt);
    void executeLetStmt(LetStmt* stmt);
    void executeBlockStmt(BlockStmt* stmt);
    void executeIfStmt(IfStmt* stmt);
    void executeWhileStmt(WhileStmt* stmt);
    void executeRunUntilStmt(RunUntilStmt* stmt);
    void executeForStmt(ForStmt* stmt);
    
    // Execute block with new environment
    void executeBlock(const std::vector<StmtPtr>& statements,
                      std::shared_ptr<Environment> environment);
    
    // Expression evaluation
    Value evaluateLiteral(LiteralExpr* expr);
    Value evaluateVariable(VariableExpr* expr);
    Value evaluateUnary(UnaryExpr* expr);
    Value evaluateBinary(BinaryExpr* expr);
    Value evaluateLogical(LogicalExpr* expr);
    Value evaluateGrouping(GroupingExpr* expr);
    Value evaluateCall(CallExpr* expr);
    Value evaluateAssign(AssignExpr* expr);
    
    // Helper methods
    void checkNumberOperand(const Token& op, const Value& operand);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);
    
    std::shared_ptr<Environment> environment_;
    std::shared_ptr<Environment> globals_;
};

// Runtime error with location info
class RuntimeError : public std::runtime_error {
public:
    Token token;
    
    RuntimeError(Token tok, const std::string& message)
        : std::runtime_error(message), token(tok) {}
};

} // namespace volt
