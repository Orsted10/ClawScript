#pragma once

#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include <memory>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <sstream>

namespace volt {

// When a function hits 'return', we use this exception to jump
// back to where the function was called. Much cleaner than checking
// every statement to see if we already returned!
class ReturnValue : public std::exception {
public:
    Value value;
    explicit ReturnValue(Value val) : value(val) {}
};

/**
 * BreakException - Exception thrown when break statement is executed
 */
class BreakException : public std::exception {};

/**
 * ContinueException - Exception thrown when continue statement is executed
 */
class ContinueException : public std::exception {};

// This runs our VoltScript programs! It walks through the
// abstract syntax tree (AST) and executes each piece. Sure, it's
// not the fastest way to interpret code, but it's straightforward.
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter();
    ~Interpreter();
    
    // Execute statements
    void execute(Stmt* stmt);
    void execute(const std::vector<StmtPtr>& statements);
    
    // Execute a block with a specific environment
    // This is public so VoltFunction can call it
    void executeBlock(const std::vector<StmtPtr>& statements,
                      std::shared_ptr<Environment> environment);
    
    // Evaluate expressions
    Value evaluate(Expr* expr);
    
    // Get current environment
    std::shared_ptr<Environment> getEnvironment() { return environment_; }
    
    // Reset interpreter state
    void reset();

    // ExprVisitor implementation
    Value visitLiteralExpr(LiteralExpr* expr) override;
    Value visitVariableExpr(VariableExpr* expr) override;
    Value visitUnaryExpr(UnaryExpr* expr) override;
    Value visitBinaryExpr(BinaryExpr* expr) override;
    Value visitLogicalExpr(LogicalExpr* expr) override;
    Value visitGroupingExpr(GroupingExpr* expr) override;
    Value visitCallExpr(CallExpr* expr) override;
    Value visitAssignExpr(AssignExpr* expr) override;
    Value visitCompoundAssignExpr(CompoundAssignExpr* expr) override;
    Value visitUpdateExpr(UpdateExpr* expr) override;
    Value visitTernaryExpr(TernaryExpr* expr) override;
    Value visitArrayExpr(ArrayExpr* expr) override;
    Value visitIndexExpr(IndexExpr* expr) override;
    Value visitIndexAssignExpr(IndexAssignExpr* expr) override;
    Value visitHashMapExpr(HashMapExpr* expr) override;
    Value visitMemberExpr(MemberExpr* expr) override;
    Value visitSetExpr(SetExpr* expr) override;
    Value visitFunctionExpr(FunctionExpr* expr) override;

    // StmtVisitor implementation
    void visitExprStmt(ExprStmt* stmt) override;
    void visitPrintStmt(PrintStmt* stmt) override;
    void visitLetStmt(LetStmt* stmt) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitRunUntilStmt(RunUntilStmt* stmt) override;
    void visitForStmt(ForStmt* stmt) override;
    void visitFnStmt(FnStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;
    void visitBreakStmt(BreakStmt* stmt) override;
    void visitContinueStmt(ContinueStmt* stmt) override;
    void visitTryStmt(TryStmt* stmt) override;
    
private:
    // Helper methods
    void checkNumberOperand(const Token& op, const Value& operand);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);
    
    // Register built-in functions (like clock(), input(), etc.)
    void defineNatives();
    
    // JSON encoding/decoding methods (NEW FOR v0.7.5)
    std::string encodeToJson(const Value& value);
    Value decodeFromJson(const std::string& jsonString);
    void encodeJsonValue(const Value& value, std::ostringstream& oss);
    
    // Recursion depth tracking for stress test protection
    int recursion_depth_ = 0;
    static const int MAX_RECURSION_DEPTH = 1000;
    
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
