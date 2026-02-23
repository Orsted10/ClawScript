#pragma once
#include "ast.h"
#include "value.h"
#include "environment.h"
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

namespace claw {

// Tree-walk interpreter
class Evaluator {
public:
    Evaluator();
    
    // Evaluate an expression and return result
    Value evaluate(Expr* expr);
    
    // Get current environment (for REPL)
    std::shared_ptr<Environment> getEnvironment() { return environment_; }
    
    // Reset environment
    void reset();

private:
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
};

// Runtime error with location info
class RuntimeError : public std::runtime_error {
public:
    Token token;
    
    RuntimeError(Token tok, const std::string& message)
        : std::runtime_error(message), token(tok) {}
};

} // namespace claw
