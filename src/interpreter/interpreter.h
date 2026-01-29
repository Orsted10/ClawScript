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
class Interpreter {
public:
    Interpreter();
    
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
    void executeFnStmt(FnStmt* stmt);
    void executeReturnStmt(ReturnStmt* stmt);
    void executeBreakStmt(BreakStmt* stmt);
    void executeContinueStmt(ContinueStmt* stmt);
    
    // Expression evaluation
    Value evaluateLiteral(LiteralExpr* expr);
    Value evaluateVariable(VariableExpr* expr);
    Value evaluateUnary(UnaryExpr* expr);
    Value evaluateBinary(BinaryExpr* expr);
    Value evaluateLogical(LogicalExpr* expr);
    Value evaluateGrouping(GroupingExpr* expr);
    Value evaluateCall(CallExpr* expr);
    Value evaluateAssign(AssignExpr* expr);
    Value evaluateCompoundAssign(CompoundAssignExpr* expr);
    Value evaluateUpdate(UpdateExpr* expr);
    Value evaluateTernary(TernaryExpr* expr);
    
    // ARRAY EVALUATION - Added!
    Value evaluateArray(ArrayExpr* expr);
    Value evaluateIndex(IndexExpr* expr);
    Value evaluateIndexAssign(IndexAssignExpr* expr);
    Value evaluateMember(MemberExpr* expr);
    
    // HASH MAP EVALUATION - Added!
    Value evaluateHashMap(HashMapExpr* expr);
    
    // Helper methods
    void checkNumberOperand(const Token& op, const Value& operand);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);
    
    // Register built-in functions (like clock(), input(), etc.)
    void defineNatives();
    
    // JSON encoding/decoding methods (NEW FOR v0.7.5)
    std::string encodeToJson(const Value& value);
    Value decodeFromJson(const std::string& jsonString);
    void encodeJsonValue(const Value& value, std::ostringstream& oss);
    
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
