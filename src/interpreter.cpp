#include "interpreter.h"
#include "callable.h"
#include <cmath>
#include <iostream>
#include <chrono>

namespace volt {

Interpreter::Interpreter() 
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {
    // Register built-in functions
    defineNatives();
}

void Interpreter::reset() {
    environment_ = std::make_shared<Environment>();
    globals_ = environment_;
    defineNatives();
}

// Register native functions (built into the language)
void Interpreter::defineNatives() {
    // clock() - returns current time in seconds
    globals_->define("clock", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return static_cast<double>(millis) / 1000.0;
        },
        "clock"
    ));
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
    } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
        executeForStmt(forStmt);
    } else if (auto* fnStmt = dynamic_cast<FnStmt*>(stmt)) {
        executeFnStmt(fnStmt);
    } else if (auto* returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        executeReturnStmt(returnStmt);
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

void Interpreter::executeFnStmt(FnStmt* stmt) {
    // Create a function object that captures the current environment
    // This is what makes closures work!
    auto function = std::make_shared<VoltFunction>(stmt, environment_);
    
    // Define the function in the current scope
    // Note: We define it AFTER creating the closure, but that's okay
    // because the function name isn't in scope inside its own body
    // (unless you reference it for recursion, which we handle specially)
    environment_->define(stmt->name, function);
}

void Interpreter::executeReturnStmt(ReturnStmt* stmt) {
    Value value = nullptr;
    
    if (stmt->value) {
        value = evaluate(stmt->value.get());
    }
    
    // Throw a special exception to unwind the call stack
    // This is caught in VoltFunction::call()
    throw ReturnValue(value);
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
        
        case TokenType::Slash:
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            return asNumber(left) / asNumber(right);
        
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
    
    // Short-circuit evaluation
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
    // Evaluate the callee (the thing being called)
    Value callee = evaluate(expr->callee.get());
    
    // Evaluate all the arguments
    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg.get()));
    }
    
    // Make sure it's actually a function
    if (!isCallable(callee)) {
        throw RuntimeError(
            Token(TokenType::LeftParen, "(", 0),
            "Can only call functions and classes"
        );
    }
    
    auto function = std::get<std::shared_ptr<Callable>>(callee);
    
    // Check arity (number of arguments)
    if (static_cast<int>(arguments.size()) != function->arity()) {
        throw RuntimeError(
            Token(TokenType::LeftParen, "(", 0),
            "Expected " + std::to_string(function->arity()) + 
            " arguments but got " + std::to_string(arguments.size())
        );
    }
    
    // Call the function!
    return function->call(*this, arguments);
}

Value Interpreter::evaluateAssign(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // If variable doesn't exist, create it (implicit declaration)
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
