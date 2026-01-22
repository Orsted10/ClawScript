#include "evaluator.h"
#include <cmath>
#include <sstream>

namespace volt {

Evaluator::Evaluator() 
    : environment_(std::make_shared<Environment>()) {}

void Evaluator::reset() {
    environment_ = std::make_shared<Environment>();
}

Value Evaluator::evaluate(Expr* expr) {
    if (auto* lit = dynamic_cast<LiteralExpr*>(expr)) {
        return evaluateLiteral(lit);
    }
    
    if (auto* var = dynamic_cast<VariableExpr*>(expr)) {
        return evaluateVariable(var);
    }
    
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
        return evaluateUnary(unary);
    }
    
    if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    }
    
    if (auto* logical = dynamic_cast<LogicalExpr*>(expr)) {
        return evaluateLogical(logical);
    }
    
    if (auto* group = dynamic_cast<GroupingExpr*>(expr)) {
        return evaluateGrouping(group);
    }
    
    if (auto* call = dynamic_cast<CallExpr*>(expr)) {
        return evaluateCall(call);
    }
    
    if (auto* assign = dynamic_cast<AssignExpr*>(expr)) {
        return evaluateAssign(assign);
    }
    
    throw std::runtime_error("Unknown expression type");
}

Value Evaluator::evaluateLiteral(LiteralExpr* expr) {
    switch (expr->type) {
        case LiteralExpr::Type::Number:
            return expr->numberValue;
        
        case LiteralExpr::Type::String:
            return expr->stringValue;
        
        case LiteralExpr::Type::Bool:
            return expr->boolValue;
        
        case LiteralExpr::Type::Nil:
            return nullptr;
    }
    
    return nullptr;
}

Value Evaluator::evaluateVariable(VariableExpr* expr) {
    try {
        return environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(Token(TokenType::Identifier, expr->name, 0), e.what());
    }
}

Value Evaluator::evaluateUnary(UnaryExpr* expr) {
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::Minus:
            checkNumberOperand(expr->op, right);
            return -asNumber(right);
        
        case TokenType::Bang:
            return !isTruthy(right);
        
        default:
            throw RuntimeError(expr->op, "Unknown unary operator");
    }
}

Value Evaluator::evaluateBinary(BinaryExpr* expr) {
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        // Arithmetic
        case TokenType::Plus:
            if (isNumber(left) && isNumber(right)) {
                return asNumber(left) + asNumber(right);
            }
            if (isString(left) && isString(right)) {
                return asString(left) + asString(right);
            }
            throw RuntimeError(expr->op, "Operands must be two numbers or two strings");
        
        case TokenType::Minus:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) - asNumber(right);
        
        case TokenType::Star:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) * asNumber(right);
        
        case TokenType::Slash:
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            return asNumber(left) / asNumber(right);
        
        case TokenType::Percent:
            checkNumberOperands(expr->op, left, right);
            return std::fmod(asNumber(left), asNumber(right));
        
        // Comparison
        case TokenType::Greater:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) > asNumber(right);
        
        case TokenType::GreaterEqual:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) >= asNumber(right);
        
        case TokenType::Less:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) < asNumber(right);
        
        case TokenType::LessEqual:
            checkNumberOperands(expr->op, left, right);
            return asNumber(left) <= asNumber(right);
        
        // Equality
        case TokenType::EqualEqual:
            return isEqual(left, right);
        
        case TokenType::BangEqual:
            return !isEqual(left, right);
        
        default:
            throw RuntimeError(expr->op, "Unknown binary operator");
    }
}

Value Evaluator::evaluateLogical(LogicalExpr* expr) {
    Value left = evaluate(expr->left.get());
    
    // Short-circuit evaluation
    if (expr->op.type == TokenType::Or) {
        if (isTruthy(left)) return left;
    } else { // AND
        if (!isTruthy(left)) return left;
    }
    
    return evaluate(expr->right.get());
}

Value Evaluator::evaluateGrouping(GroupingExpr* expr) {
    return evaluate(expr->expr.get());
}

Value Evaluator::evaluateCall(CallExpr* expr) {
    // For now, throw error (functions not implemented yet)
    throw RuntimeError(Token(TokenType::LeftParen, "(", 0), 
                      "Function calls not yet implemented");
}

Value Evaluator::evaluateAssign(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // Variable doesn't exist, define it
        environment_->define(expr->name, value);
    }
    
    return value;
}

void Evaluator::checkNumberOperand(const Token& op, const Value& operand) {
    if (isNumber(operand)) return;
    throw RuntimeError(op, "Operand must be a number");
}

void Evaluator::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (isNumber(left) && isNumber(right)) return;
    throw RuntimeError(op, "Operands must be numbers");
}

} // namespace volt
