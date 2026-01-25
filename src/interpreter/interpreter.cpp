#include "interpreter.h"
#include "callable.h"
#include "array.h"  // NEW!
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <cmath>

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
    
    // len(value) - returns length of string or array  // UPDATED!
    globals_->define("len", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                return static_cast<double>(asString(args[0]).length());
            }
            if (isArray(args[0])) {  // NEW!
                return static_cast<double>(asArray(args[0])->length());
            }
            throw std::runtime_error("len() requires a string or array argument");
        },
        "len"
    ));
    
    // str(value) - convert to string
    globals_->define("str", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return valueToString(args[0]);
        },
        "str"
    ));
    
    // num(value) - convert to number
    globals_->define("num", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isNumber(args[0])) return args[0];
            if (isString(args[0])) {
                try {
                    return std::stod(asString(args[0]));
                } catch (...) {
                    throw std::runtime_error("Cannot convert string to number: " + asString(args[0]));
                }
            }
            if (isBool(args[0])) {
                return asBool(args[0]) ? 1.0 : 0.0;
            }
            throw std::runtime_error("Cannot convert to number");
        },
        "num"
    ));
    
    // input(prompt) - read line from stdin
    globals_->define("input", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isString(args[0])) {
                std::cout << asString(args[0]);
            }
            std::string line;
            std::getline(std::cin, line);
            return line;
        },
        "input"
    ));
    
    // readFile(path) - read entire file as string
    globals_->define("readFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("readFile() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file: " + asString(args[0]));
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        },
        "readFile"
    ));
    
    // writeFile(path, content) - write string to file (overwrites)
    globals_->define("writeFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("writeFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]));
            if (!file) {
                throw std::runtime_error("Could not open file for writing: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "writeFile"
    ));
    
    // appendFile(path, content) - append string to file
    globals_->define("appendFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1])) {
                throw std::runtime_error("appendFile() requires string path and content");
            }
            std::ofstream file(asString(args[0]), std::ios::app);
            if (!file) {
                throw std::runtime_error("Could not open file for appending: " + asString(args[0]));
            }
            file << asString(args[1]);
            return true;
        },
        "appendFile"
    ));
    
    // fileExists(path) - check if file exists
    globals_->define("fileExists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) {
                throw std::runtime_error("fileExists() requires a string path");
            }
            std::ifstream file(asString(args[0]));
            return file.good();
        },
        "fileExists"
    ));
    
    // toUpper(str) - convert string to uppercase
    globals_->define("toUpper", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toUpper() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return s;
        },
        "toUpper"
    ));
    
    // toLower(str) - convert string to lowercase
    globals_->define("toLower", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("toLower() requires a string");
            std::string s = asString(args[0]);
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        },
        "toLower"
    ));
    
    // type(val) - get type of value as string
    globals_->define("type", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            const Value& v = args[0];
            if (isNil(v)) return "nil";
            if (isBool(v)) return "bool";
            if (isNumber(v)) return "number";
            if (isString(v)) return "string";
            if (isCallable(v)) return "function";
            if (isArray(v)) return "array";
            return "unknown";
        },
        "type"
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
    } else if (auto* breakStmt = dynamic_cast<BreakStmt*>(stmt)) {
        executeBreakStmt(breakStmt);
    } else if (auto* continueStmt = dynamic_cast<ContinueStmt*>(stmt)) {
        executeContinueStmt(continueStmt);
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
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
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
        
        // Loop with break/continue support
        while (checkCondition()) {
            try {
                execute(stmt->body.get());
            } catch (const ContinueException&) {
                // Continue - execute increment and check condition
            } catch (const BreakException&) {
                break; // Exit the loop
            }
            
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
    
    if (auto* compound = dynamic_cast<CompoundAssignExpr*>(expr)) {
        return evaluateCompoundAssign(compound);
    }
    
    if (auto* update = dynamic_cast<UpdateExpr*>(expr)) {
        return evaluateUpdate(update);
    }
    
    if (auto* ternary = dynamic_cast<TernaryExpr*>(expr)) {
        return evaluateTernary(ternary);
    }
    
    // ========================================
    // ARRAY EXPRESSIONS - NEW!
    // ========================================
    
    if (auto* array = dynamic_cast<ArrayExpr*>(expr)) {
        return evaluateArray(array);
    }
    
    if (auto* index = dynamic_cast<IndexExpr*>(expr)) {
        return evaluateIndex(index);
    }
    
    if (auto* indexAssign = dynamic_cast<IndexAssignExpr*>(expr)) {
        return evaluateIndexAssign(indexAssign);
    }
    
    if (auto* member = dynamic_cast<MemberExpr*>(expr)) {
        return evaluateMember(member);
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
        throw RuntimeError(expr->token, e.what());
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
            // Type coercion: string + number or number + string
            if (isString(left) && isNumber(right)) {
                return asString(left) + valueToString(right);
            }
            if (isNumber(left) && isString(right)) {
                return valueToString(left) + asString(right);
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
            expr->token,
            "Can only call functions and classes"
        );
    }
    
    auto function = std::get<std::shared_ptr<Callable>>(callee);
    
    // Check arity (number of arguments)
    if (static_cast<int>(arguments.size()) != function->arity()) {
        throw RuntimeError(
            expr->token,
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

Value Interpreter::evaluateCompoundAssign(CompoundAssignExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    Value operand = evaluate(expr->value.get());
    Value result;
    
    switch (expr->op.type) {
        case TokenType::PlusEqual:
            if (isNumber(current) && isNumber(operand)) {
                result = asNumber(current) + asNumber(operand);
            } else if (isString(current) && isString(operand)) {
                result = asString(current) + asString(operand);
            } else if (isString(current) && isNumber(operand)) {
                result = asString(current) + valueToString(operand);
            } else {
                throw RuntimeError(expr->op, "Operands must be compatible for +=");
            }
            break;
        case TokenType::MinusEqual:
            checkNumberOperands(expr->op, current, operand);
            result = asNumber(current) - asNumber(operand);
            break;
        case TokenType::StarEqual:
            checkNumberOperands(expr->op, current, operand);
            result = asNumber(current) * asNumber(operand);
            break;
        case TokenType::SlashEqual:
            checkNumberOperands(expr->op, current, operand);
            if (asNumber(operand) == 0.0) {
                throw RuntimeError(expr->op, "Division by zero");
            }
            result = asNumber(current) / asNumber(operand);
            break;
        default:
            throw RuntimeError(expr->op, "Unknown compound assignment operator");
    }
    
    try {
        environment_->assign(expr->name, result);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    return result;
}

Value Interpreter::evaluateUpdate(UpdateExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    if (!isNumber(current)) {
        throw RuntimeError(expr->op, "Operand must be a number for increment/decrement");
    }
    
    double oldValue = asNumber(current);
    double newValue;
    if (expr->op.type == TokenType::PlusPlus) {
        newValue = oldValue + 1;
    } else {
        newValue = oldValue - 1;
    }
    
    try {
        environment_->assign(expr->name, newValue);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
    
    // Return old value for postfix, new value for prefix
    return expr->prefix ? newValue : oldValue;
}

Value Interpreter::evaluateTernary(TernaryExpr* expr) {
    if (isTruthy(evaluate(expr->condition.get()))) {
        return evaluate(expr->thenBranch.get());
    }
    return evaluate(expr->elseBranch.get());
}

// ========================================
// ARRAY EVALUATION - NEW METHODS!
// ========================================

Value Interpreter::evaluateArray(ArrayExpr* expr) {
    std::vector<Value> elements;
    
    // Evaluate all element expressions
    for (const auto& elem : expr->elements) {
        elements.push_back(evaluate(elem.get()));
    }
    
    // Create and return array
    return std::make_shared<VoltArray>(elements);
}

Value Interpreter::evaluateIndex(IndexExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    
    // Must be an array
    if (!isArray(object)) {
        throw RuntimeError(expr->token, "Can only index arrays");
    }
    
    // Index must be a number
    if (!isNumber(index)) {
        throw RuntimeError(expr->token, "Array index must be a number");
    }
    
    auto array = asArray(object);
    int idx = static_cast<int>(asNumber(index));
    
    // Check bounds
    if (idx < 0 || idx >= array->length()) {
        throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
    }
    
    return array->get(idx);
}

Value Interpreter::evaluateIndexAssign(IndexAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    Value value = evaluate(expr->value.get());
    
    // Must be an array
    if (!isArray(object)) {
        throw RuntimeError(expr->token, "Can only index arrays");
    }
    
    // Index must be a number
    if (!isNumber(index)) {
        throw RuntimeError(expr->token, "Array index must be a number");
    }
    
    auto array = asArray(object);
    int idx = static_cast<int>(asNumber(index));
    
    // Check bounds
    if (idx < 0 || idx >= array->length()) {
        throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
    }
    
    array->set(idx, value);
    return value;
}

Value Interpreter::evaluateMember(MemberExpr* expr) {
    Value object = evaluate(expr->object.get());
    
    // Must be an array
    if (!isArray(object)) {
        throw RuntimeError(expr->token, "Only arrays have members");
    }
    
    auto array = asArray(object);
    
    // Handle array.length
    if (expr->member == "length") {
        return static_cast<double>(array->length());
    }
    
    // Handle array.push - return a callable
    if (expr->member == "push") {
        return std::make_shared<NativeFunction>(
            1,
            [array](const std::vector<Value>& args) -> Value {
                array->push(args[0]);
                return nullptr; // returns nil
            },
            "push"
        );
    }
    
    // Handle array.pop
    if (expr->member == "pop") {
        return std::make_shared<NativeFunction>(
            0,
            [array](const std::vector<Value>&) -> Value {
                return array->pop();
            },
            "pop"
        );
    }
    
    // Handle array.reverse
    if (expr->member == "reverse") {
        return std::make_shared<NativeFunction>(
            0,
            [array](const std::vector<Value>&) -> Value {
                array->reverse();
                return nullptr;
            },
            "reverse"
        );
    }
    
    throw RuntimeError(expr->token, "Unknown array member: " + expr->member);
}

void Interpreter::executeBreakStmt(BreakStmt*) {
    throw BreakException();
}

void Interpreter::executeContinueStmt(ContinueStmt*) {
    throw ContinueException();
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
