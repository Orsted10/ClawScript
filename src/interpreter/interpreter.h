#pragma once

#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "stack_trace.h"
#include "module.h"
#include <memory>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <sstream>

namespace claw {

// Runtime error with location info
class RuntimeError : public ClawError {
public:
    Token token;
    std::vector<StackFrame> stack_trace;
    
    RuntimeError(Token tok, ErrorCode code, const std::string& message)
        : ClawError(code, message), token(tok) {}

    RuntimeError(Token tok, ErrorCode code, const std::string& message, std::vector<StackFrame> trace)
        : ClawError(code, message), token(tok), stack_trace(std::move(trace)) {}
};

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

    // Call stack for tracing
    CallStack& getCallStack() { return call_stack_; }
    
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
    Value visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) override;
    Value visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) override;
    Value visitUpdateExpr(UpdateExpr* expr) override;
    Value visitUpdateMemberExpr(UpdateMemberExpr* expr) override;
    Value visitUpdateIndexExpr(UpdateIndexExpr* expr) override;
    Value visitTernaryExpr(TernaryExpr* expr) override;
    Value visitArrayExpr(ArrayExpr* expr) override;
    Value visitIndexExpr(IndexExpr* expr) override;
    Value visitIndexAssignExpr(IndexAssignExpr* expr) override;
    Value visitHashMapExpr(HashMapExpr* expr) override;
    Value visitMemberExpr(MemberExpr* expr) override;
    Value visitSetExpr(SetExpr* expr) override;
    Value visitThisExpr(ThisExpr* expr) override;
    Value visitSuperExpr(SuperExpr* expr) override;
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
    void visitThrowStmt(ThrowStmt* stmt) override;
    void visitImportStmt(ImportStmt* stmt) override;
    void visitClassStmt(ClassStmt* stmt) override;
    void visitSwitchStmt(SwitchStmt* stmt) override;
    
    // Get global environment
    std::shared_ptr<Environment> getGlobals() const { return globals_; }
    
    // Get current environment
    std::shared_ptr<Environment> getEnvironment() const { return environment_; }
    
private:
    // Helper methods
    void checkNumberOperand(const Token& op, const Value& operand);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);
    
    // Register built-in functions (like clock(), input(), etc.)
    void defineNatives();

    // Helper to throw runtime errors with stack trace
    [[noreturn]] void throwRuntimeError(const Token& token, ErrorCode code, const std::string& message) {
        throw RuntimeError(token, code, message, call_stack_.get_frames());
    }
    
    // Recursion depth tracking for stress test protection
    int recursion_depth_ = 0;
    static const int MAX_RECURSION_DEPTH = 1000;
    
    CallStack call_stack_;
    std::shared_ptr<Environment> environment_;
    std::shared_ptr<Environment> globals_;
    ModuleManager module_manager_;
};

} // namespace claw
