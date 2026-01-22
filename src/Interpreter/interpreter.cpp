#include "interpreter.h"
#include <cmath>
#include <iostream>

namespace volt {

Interpreter::Interpreter() 
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {}

void Interpreter::reset() {
    environment_ = std::make_shared<Environment>();
    globals_ = environment_;
}

// ========================================
// STATEMENT EXECUTION
// ========================================

void Interpreter::execute(Stmt* stmt) {
    if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        executeExprStmt(exprStmt);
    } else if (auto* printStmt = dynamic_cast<PrintStmt*>(stmt)) {
        executePrintStmt(printStmt);
    } else if (auto* letStmt = dynamic_cast<LetStmt*>(stmt)) {
        executeLetStmt(letStmt);
    } else if (auto* blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        executeBlockStmt(blockStmt);
    } else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        executeIfStmt(ifStmt);
    } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        executeWhileStmt(whileStmt);
    } else if (auto* runUntilStmt = dynamic_cast<RunUntilStmt*>(stmt)) {
        executeRunUntilStmt(runUntilStmt);
    } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
        executeForStmt(forStmt);
    } else {
        throw std::runtime_error("Unknown statement type");
    }
}

void Interpreter::execute(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::executeExprStmt(ExprStmt* stmt) {
    evaluate(stmt->expr.get());
}

void Interpreter::executePrintStmt(PrintStmt* stmt) {
    Value value = evaluate(stmt->expr.get());
    std::cout << valueToString(value) << "\n";
}

void Interpreter::executeLetStmt(LetStmt* stmt) {
    Value value = nullptr;
    
    if (stmt->initializer) {
        value = evaluate(stmt->initializer.get());
    }
    
    environment_->define(stmt->name, value);
}

void Interpreter::executeBlockStmt(BlockStmt* stmt) {
    executeBlock(stmt->statements, 
                 std::make_shared<Environment>(environment_));
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements,
                               std::shared_ptr<Environment> environment) {
    std::shared_ptr<Environment> previous = environment_;
    
    try {
        environment_ = environment;
        
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
        
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

void Interpreter::executeIfStmt(IfStmt* stmt) {
    Value condition = evaluate(stmt->condition.get());
    
    if (isTruthy(condition)) {
        execute(stmt->thenBranch.get());
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch.get());
    }
}

void Interpreter::executeWhileStmt(WhileStmt* stmt) {
    while (isTruthy(evaluate(stmt->condition.get()))) {
        execute(stmt->body.get());
    }
}

void Interpreter::executeRunUntilStmt(RunUntilStmt* stmt) {
    do {
        execute(stmt->body.get());
    } while (!isTruthy(evaluate(stmt->condition.get())));
}

void Interpreter::executeForStmt(ForStmt* stmt) {
    // Create new scope for loop
    auto loopEnv = std::make_shared<Environment>(environment_);
    auto previous = environment_;
    
    try {
        environment_ = loopEnv;
        
        // Execute initializer
        if (stmt->initializer) {
            execute(stmt->initializer.get());
        }
        
        // Condition (default to true if omitted)
        auto checkCondition = [&]() {
            if (stmt->condition) {
                return isTruthy(evaluate(stmt->condition.get()));
            }
            return true;
        };
        
        // Loop
        while (checkCondition()) {
            execute(stmt->body.get());
            
            // Execute increment
            if (stmt->increment) {
                evaluate(stmt->increment.get());
            }
        }
        
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

// ========================================
// EXPRESSION EVALUATION
// ========================================

Value Interpreter::evaluate(Expr* expr) {
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

Value Interpreter::evaluateLiteral(LiteralExpr* expr) {
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

Value Interpreter::evaluateVariable(VariableExpr* expr) {
    try {
        return environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(Token(TokenType::Identifier, expr->name, 0), e.what());
    }
}

Value Interpreter::evaluateUnary(UnaryExpr* expr) {
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

Value Interpreter::evaluateBinary(BinaryExpr* expr) {
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
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
        
        case TokenType::Slash: {
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            double result = asNumber(left) / asNumber(right);
            
            // Truncate towards zero (integer division behavior)
            if (std::floor(result) == result) {
                return result;  // Already an integer
            }
            return result;
        }
        
        case TokenType::Percent:
            checkNumberOperands(expr->op, left, right);
            return std::fmod(asNumber(left), asNumber(right));
        
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
        
        case TokenType::EqualEqual:
            return isEqual(left, right);
        
        case TokenType::BangEqual:
            return !isEqual(left, right);
        
        default:
            throw RuntimeError(expr->op, "Unknown binary operator");
    }
}

Value Interpreter::evaluateLogical(LogicalExpr* expr) {
    Value left = evaluate(expr->left.get());
    
    if (expr->op.type == TokenType::Or) {
        if (isTruthy(left)) return left;
    } else {
        if (!isTruthy(left)) return left;
    }
    
    return evaluate(expr->right.get());
}

Value Interpreter::evaluateGrouping(GroupingExpr* expr) {
    return evaluate(expr->expr.get());
}

Value Interpreter::evaluateCall(CallExpr* expr) {
    throw RuntimeError(Token(TokenType::LeftParen, "(", 0), 
                      "Function calls not yet implemented");
}

Value Interpreter::evaluateAssign(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // Variable doesn't exist, define it
        environment_->define(expr->name, value);
    }
    
    return value;
}

void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (isNumber(operand)) return;
    throw RuntimeError(op, "Operand must be a number");
}

void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (isNumber(left) && isNumber(right)) return;
    throw RuntimeError(op, "Operands must be numbers");
}

} // namespace volt
