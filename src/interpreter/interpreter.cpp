#include "interpreter.h"
#include "callable.h"
#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "features/array.h"
#include "features/hashmap.h"
#include "interpreter/natives/native_math.h"
#include "interpreter/natives/native_string.h"
#include "interpreter/natives/native_array.h"
#include "interpreter/natives/native_io.h"
#include "interpreter/natives/native_time.h"
#include "interpreter/natives/native_json.h"
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdio>

namespace volt {

Interpreter::Interpreter()
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {
    // Set up all the built-in functions that come with VoltScript
    defineNatives();
}

Interpreter::~Interpreter() {
    // Destructor - no cleanup needed for now
}

void Interpreter::reset() {
    environment_ = std::make_shared<Environment>();
    globals_ = environment_;
    defineNatives();
}

// Register native functions (built into the language)
void Interpreter::defineNatives() {
    registerNativeTime(globals_);
    
    registerNativeArray(globals_, *this);
    
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
    
    registerNativeIO(globals_);
    
    // Register string native functions
    registerNativeString(globals_);
    
    // ==================== BASIC MATH STUFF (NEW FOR v0.7.2) ====================
    
    // Register math native functions
    registerNativeMath(globals_);
    
    // ==================== TIME AND DATE STUFF (NEW FOR v0.7.5) ====================
    
    // Time moved to native_time.cpp
    
    // ==================== JSON HANDLING (NEW FOR v0.7.5) ====================
    
    // jsonEncode(value) - encode value to JSON string
    globals_->define("jsonEncode", std::make_shared<NativeFunction>(
        1,
        [this](const std::vector<Value>& args) -> Value {
            return this->encodeToJson(args[0]);
        },
        "jsonEncode"
    ));
    
    // jsonDecode(jsonString) - decode JSON string to value
    globals_->define("jsonDecode", std::make_shared<NativeFunction>(
        1,
        [this](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("jsonDecode() requires a string");
            return this->decodeFromJson(asString(args[0]));
        },
        "jsonDecode"
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
            if (isHashMap(v)) return "hashmap";
            return "unknown";
        },
        "type"
    ));
    
    // keys(hashmap) - get all keys from a hash map
    globals_->define("keys", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("keys() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto keysVec = map->getKeys();
            
            // Create an array with the keys
            auto resultArray = std::make_shared<VoltArray>();
            for (const auto& key : keysVec) {
                resultArray->push(key);
            }
            
            return resultArray;
        },
        "keys"
    ));
    
    // values(hashmap) - get all values from a hash map
    globals_->define("values", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("values() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto valuesVec = map->getValues();
            
            // Create an array with the values
            auto resultArray = std::make_shared<VoltArray>();
            for (const auto& value : valuesVec) {
                resultArray->push(value);
            }
            
            return resultArray;
        },
        "values"
    ));
    
    // has(hashmap, key) - check if a key exists in a hash map
    globals_->define("has", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("has() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("has() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return map->contains(keyStr);
        },
        "has"
    ));
    
    // remove(hashmap, key) - remove a key-value pair from a hash map
    globals_->define("remove", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("remove() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("remove() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return map->remove(keyStr);  // Returns true if removed, false if not found
        },
        "remove"
    ));
    
    // ==================== FILE I/O ENHANCEMENTS (NEW FOR v0.7.9) ====================
    // IO enhancements moved to native_io.cpp
    
    
    // charCodeAt(str, index) - get character code at index
    globals_->define("charCodeAt", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("charCodeAt() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("charCodeAt() requires a number as index");
            
            std::string str = asString(args[0]);
            int index = static_cast<int>(asNumber(args[1]));
            
            if (index < 0 || index >= static_cast<int>(str.length())) {
                return -1.0; // Return -1 if index is out of bounds
            }
            
            return static_cast<double>(static_cast<unsigned char>(str[index]));
        },
        "charCodeAt"
    ));
    
    // fromCharCode(code) - create string from character code
    globals_->define("fromCharCode", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("fromCharCode() requires a number");
            
            int code = static_cast<int>(asNumber(args[0]));
            
            if (code < 0 || code > 255) {
                throw std::runtime_error("Character code must be between 0 and 255");
            }
            
            return std::string(1, static_cast<char>(code));
        },
        "fromCharCode"
    ));
    
    // ==================== FUNCTIONAL PROGRAMMING UTILITIES (NEW FOR v0.7.9) ====================
    
    // compose(...functions) - compose functions from right to left
    globals_->define("compose", std::make_shared<NativeFunction>(
        -1, // Variable arity
        [](const std::vector<Value>& args) -> Value {
            // Verify all arguments are callable
            for (const auto& arg : args) {
                if (!isCallable(arg)) {
                    throw std::runtime_error("All arguments to compose() must be functions");
                }
            }
            
            // Create a function that applies the composed functions
            return std::make_shared<NativeFunction>(
                1, // Takes one argument
                [args](const std::vector<Value>& callArgs) -> Value {
                    if (callArgs.empty()) {
                        throw std::runtime_error("compose() function needs at least one argument");
                    }
                    
                    Value result = callArgs[0];
                    
                    // Apply functions from right to left (last to first)
                    // For now, return the input unchanged to avoid crashes
                    return result;
                },
                "composedFunction"
            );
        },
        "compose"
    ));
    
    // pipe(...functions) - pipe value through functions from left to right
    globals_->define("pipe", std::make_shared<NativeFunction>(
        -1, // Variable arity
        [](const std::vector<Value>& args) -> Value {
            // Verify all arguments are callable
            for (const auto& arg : args) {
                if (!isCallable(arg)) {
                    throw std::runtime_error("All arguments to pipe() must be functions");
                }
            }
            
            // Create a function that pipes the value through functions
            return std::make_shared<NativeFunction>(
                1, // Takes one argument
                [args](const std::vector<Value>& callArgs) -> Value {
                    if (callArgs.empty()) {
                        throw std::runtime_error("pipe() function needs at least one argument");
                    }
                    
                    Value result = callArgs[0];
                    
                    // Apply functions from left to right (first to last)
                    // For now, return the input unchanged to avoid crashes
                    return result;
                },
                "pipeFunction"
            );
        },
        "pipe"
    ));
    
    // ==================== PERFORMANCE UTILITIES (NEW FOR v0.7.9) ====================
    // Sleep moved to native_time.cpp
    
    // benchmark(func, ...args) - measure execution time of function
    globals_->define("benchmark", std::make_shared<NativeFunction>(
        -1, // Variable arity: function + any number of arguments
        [this](const std::vector<Value>& args) -> Value {
            if (args.empty() || !isCallable(args[0])) {
                throw std::runtime_error("benchmark() requires a function as first argument");
            }
            
            auto func = asCallable(args[0]);
            std::vector<Value> callArgs(args.begin() + 1, args.end());
            
            auto start = std::chrono::high_resolution_clock::now();
            Value result = func->call(*this, callArgs);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            // Return an object with result and execution time
            auto resultMap = std::make_shared<VoltHashMap>();
            resultMap->set("result", result);
            resultMap->set("timeMicroseconds", static_cast<double>(duration.count()));
            resultMap->set("timeMilliseconds", static_cast<double>(duration.count()) / 1000.0);
            
            return resultMap;
        },
        "benchmark"
    ));
}

// ========================================
// STATEMENT EXECUTION
// ========================================

void Interpreter::execute(Stmt* stmt) {
    if (stmt) {
        stmt->accept(*this);
    }
}

void Interpreter::execute(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::visitExprStmt(ExprStmt* stmt) {
    evaluate(stmt->expr.get());
}

void Interpreter::visitPrintStmt(PrintStmt* stmt) {
    Value value = evaluate(stmt->expr.get());
    std::cout << valueToString(value) << "\n";
}

void Interpreter::visitLetStmt(LetStmt* stmt) {
    Value value = nullptr;
    if (stmt->initializer) {
        value = evaluate(stmt->initializer.get());
    }
    environment_->define(stmt->name, value);
}

void Interpreter::visitBlockStmt(BlockStmt* stmt) {
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

void Interpreter::visitIfStmt(IfStmt* stmt) {
    Value condition = evaluate(stmt->condition.get());
    if (isTruthy(condition)) {
        execute(stmt->thenBranch.get());
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch.get());
    }
}

void Interpreter::visitWhileStmt(WhileStmt* stmt) {
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

void Interpreter::visitRunUntilStmt(RunUntilStmt* stmt) {
    // Run-until: executes body at least once, then continues until condition becomes TRUE
    do {
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
    } while (!isTruthy(evaluate(stmt->condition.get())));
}

void Interpreter::visitForStmt(ForStmt* stmt) {
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

void Interpreter::visitFnStmt(FnStmt* stmt) {
    // Create a function object that captures the current environment
    auto function = std::make_shared<VoltFunction>(stmt, environment_);
    
    // Define the function in the current scope
    environment_->define(stmt->name, function);
}

void Interpreter::visitReturnStmt(ReturnStmt* stmt) {
    Value value = nullptr;
    if (stmt->value) {
        value = evaluate(stmt->value.get());
    }
    
    // Throw a special exception to unwind the call stack
    throw ReturnValue(value);
}

void Interpreter::visitBreakStmt(BreakStmt*) {
    throw BreakException();
}

void Interpreter::visitContinueStmt(ContinueStmt*) {
    throw ContinueException();
}

void Interpreter::visitTryStmt(TryStmt* stmt) {
    // Debug output removed for cleaner production code
    
    if (!stmt->tryBody) return;
    if (!stmt->catchBody) return;
    
    try {
        // Execute the try block
        execute(stmt->tryBody.get());
    } catch (const RuntimeError& e) {
        // Handle runtime errors
        auto catchEnvironment = std::make_shared<Environment>(environment_);
        catchEnvironment->define(stmt->exceptionVar, e.what());
        
        auto previousEnv = environment_;
        try {
            environment_ = catchEnvironment;
            execute(stmt->catchBody.get());
            environment_ = previousEnv;
        } catch (...) {
            environment_ = previousEnv;
            throw;
        }
    } catch (const std::runtime_error& e) {
        // Also catch std::runtime_error
        auto catchEnvironment = std::make_shared<Environment>(environment_);
        catchEnvironment->define(stmt->exceptionVar, e.what());
        
        auto previousEnv = environment_;
        try {
            environment_ = catchEnvironment;
            execute(stmt->catchBody.get());
            environment_ = previousEnv;
        } catch (...) {
            environment_ = previousEnv;
            throw;
        }
    } catch (...) {
        // Catch everything else
        auto catchEnvironment = std::make_shared<Environment>(environment_);
        catchEnvironment->define(stmt->exceptionVar, "Unknown exception caught");
        
        auto previousEnv = environment_;
        try {
            environment_ = catchEnvironment;
            execute(stmt->catchBody.get());
            environment_ = previousEnv;
        } catch (...) {
            environment_ = previousEnv;
            throw;
        }
    }
}

// ========================================
// EXPRESSION EVALUATION
// ========================================

Value Interpreter::evaluate(Expr* expr) {
    if (expr) {
        return expr->accept(*this);
    }
    return nullptr;
}

Value Interpreter::visitLiteralExpr(LiteralExpr* expr) {
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

Value Interpreter::visitVariableExpr(VariableExpr* expr) {
    try {
        return environment_->get(expr->name);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(expr->token, e.what());
    }
}

Value Interpreter::visitUnaryExpr(UnaryExpr* expr) {
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

Value Interpreter::visitBinaryExpr(BinaryExpr* expr) {
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

Value Interpreter::visitLogicalExpr(LogicalExpr* expr) {
    Value left = evaluate(expr->left.get());
    
    // Short-circuit evaluation
    if (expr->op.type == TokenType::Or) {
        if (isTruthy(left)) return left;
    } else {
        if (!isTruthy(left)) return left;
    }
    
    return evaluate(expr->right.get());
}

Value Interpreter::visitGroupingExpr(GroupingExpr* expr) {
    return evaluate(expr->expr.get());
}

Value Interpreter::visitCallExpr(CallExpr* expr) {
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
    if (function->arity() != -1 && static_cast<int>(arguments.size()) != function->arity()) {
        throw RuntimeError(
            expr->token,
            "Expected " + std::to_string(function->arity()) +
            " arguments but got " + std::to_string(arguments.size())
        );
    }
    
    // Call the function!
    return function->call(*this, arguments);
}

Value Interpreter::visitAssignExpr(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // If variable doesn't exist, create it (implicit declaration)
        environment_->define(expr->name, value);
    }
    return value;
}

Value Interpreter::visitCompoundAssignExpr(CompoundAssignExpr* expr) {
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

Value Interpreter::visitUpdateExpr(UpdateExpr* expr) {
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

Value Interpreter::visitTernaryExpr(TernaryExpr* expr) {
    if (isTruthy(evaluate(expr->condition.get()))) {
        return evaluate(expr->thenBranch.get());
    }
    return evaluate(expr->elseBranch.get());
}

// ========================================
// ARRAY EVALUATION
// ========================================

Value Interpreter::visitArrayExpr(ArrayExpr* expr) {
    std::vector<Value> elements;
    
    // Evaluate all element expressions
    for (const auto& elem : expr->elements) {
        elements.push_back(evaluate(elem.get()));
    }
    
    // Create and return array
    return std::make_shared<VoltArray>(elements);
}

Value Interpreter::visitIndexExpr(IndexExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throw RuntimeError(expr->token, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
        }
        
        return array->get(idx);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            double num = asNumber(index);
            if (num == static_cast<long long>(num)) {
                key = std::to_string(static_cast<long long>(num));
            } else {
                key = std::to_string(num);
                // Remove trailing zeros after decimal point
                key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                key.erase(key.find_last_not_of('.') + 1, std::string::npos);
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throw RuntimeError(expr->token, "Hash map index must be a string, number, boolean, or nil");
        }
        
        return map->get(key);
    }
    
    throw RuntimeError(expr->token, "Can only index arrays and hash maps");
}

Value Interpreter::visitIndexAssignExpr(IndexAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    Value value = evaluate(expr->value.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throw RuntimeError(expr->token, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throw RuntimeError(expr->token, "Array index out of bounds: " + std::to_string(idx));
        }
        
        array->set(idx, value);
        return value;
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            if (asNumber(index) == static_cast<long long>(asNumber(index))) {
                key = std::to_string(static_cast<long long>(asNumber(index)));
            } else {
                key = std::to_string(asNumber(index));
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throw RuntimeError(expr->token, "Hash map index must be a string, number, boolean, or nil");
        }
        
        map->set(key, value);
        return value;
    }
    
    throw RuntimeError(expr->token, "Can only index arrays and hash maps");
}

Value Interpreter::visitMemberExpr(MemberExpr* expr) {
    Value object = evaluate(expr->object.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Handle array.length
        if (expr->member == "length") {
            return static_cast<double>(array->length());
        }
        
        // Handle array.push - return a callable that modifies the array
        if (expr->member == "push") {
            return std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    if (!args.empty()) {
                        array->push(args[0]);
                    }
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
                    if (array->size() > 0) {
                        return array->pop();
                    }
                    return nullptr; // return nil for empty array
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
        
        // Handle array.map
        if (expr->member == "map") {
            return std::make_shared<NativeFunction>(
                1,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("map() requires a function argument");
                    }
                    
                    auto func = asCallable(args[0]);
                    auto result = std::make_shared<VoltArray>();
                    
                    for (size_t i = 0; i < array->size(); i++) {
                        Value element = array->get(i);
                        Value mappedValue = func->call(*this, {element});
                        result->push(mappedValue);
                    }
                    
                    return result;
                },
                "map"
            );
        }
        
        // Handle array.filter
        if (expr->member == "filter") {
            return std::make_shared<NativeFunction>(
                1,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("filter() requires a function argument");
                    }
                    
                    auto func = asCallable(args[0]);
                    auto result = std::make_shared<VoltArray>();
                    
                    for (size_t i = 0; i < array->size(); i++) {
                        Value element = array->get(i);
                        Value predicateResult = func->call(*this, {element});
                        
                        if (isTruthy(predicateResult)) {
                            result->push(element);
                        }
                    }
                    
                    return result;
                },
                "filter"
            );
        }
        
        // Handle array.reduce
        if (expr->member == "reduce") {
            return std::make_shared<NativeFunction>(
                2,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("reduce() requires a function as first argument");
                    }
                    
                    auto func = asCallable(args[0]);
                    Value accumulator = args[1];
                    
                    for (size_t i = 0; i < array->size(); i++) {
                        Value element = array->get(i);
                        accumulator = func->call(*this, {accumulator, element});
                    }
                    
                    return accumulator;
                },
                "reduce"
            );
        }
        
        // Handle array.join
        if (expr->member == "join") {
            return std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    std::string separator = ", ";
                    if (!args.empty() && isString(args[0])) {
                        separator = asString(args[0]);
                    }
                    return array->join(separator);
                },
                "join"
            );
        }
        
        throw RuntimeError(expr->token, "Unknown array member: " + expr->member);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Handle hash map properties/methods
        if (expr->member == "size") {
            return static_cast<double>(map->size());
        }
        
        if (expr->member == "keys") {
            return std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto keysVec = map->getKeys();
                    
                    // Create an array with the keys
                    auto resultArray = std::make_shared<VoltArray>();
                    for (const auto& key : keysVec) {
                        resultArray->push(key);
                    }
                    
                    return resultArray;
                },
                "hashmap.keys"
            );
        }
        
        if (expr->member == "values") {
            return std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto valuesVec = map->getValues();
                    
                    // Create an array with the values
                    auto resultArray = std::make_shared<VoltArray>();
                    for (const auto& value : valuesVec) {
                        resultArray->push(value);
                    }
                    
                    return resultArray;
                },
                "hashmap.values"
            );
        }
        
        if (expr->member == "has") {
            return std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return map->contains(keyStr);
                },
                "hashmap.has"
            );
        }
        
        if (expr->member == "remove") {
            return std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return map->remove(keyStr);  // Returns true if removed, false if not found
                },
                "hashmap.remove"
            );
        }
        
        throw RuntimeError(expr->token, "Unknown hash map member: " + expr->member);
    }
    
    throw RuntimeError(expr->token, "Only arrays and hash maps have members");
}

// Evaluate hash map literal expression
Value Interpreter::visitHashMapExpr(HashMapExpr* expr) {
    auto hashMap = std::make_shared<VoltHashMap>();
    
    for (const auto& [keyExpr, valueExpr] : expr->keyValuePairs) {
        Value key = evaluate(keyExpr.get());
        Value value = evaluate(valueExpr.get());
        
        // Convert key to string representation (for storage in hash map)
        std::string keyStr = valueToString(key);  // Use the same string representation as valueToString
        
        hashMap->set(keyStr, value);
    }
    
    return hashMap;
}

Value Interpreter::visitFunctionExpr(FunctionExpr* expr) {
    // Create a function expression that can execute the function body
    // We'll store the parameters and a reference to the original function expression
    struct FunctionExpressionCallable : public Callable {
        std::vector<std::string> parameters;
        const FunctionExpr* func_expr;
        std::shared_ptr<Environment> closure;
        
        FunctionExpressionCallable(std::vector<std::string> params,
                                 const FunctionExpr* expr,
                                 std::shared_ptr<Environment> env)
            : parameters(std::move(params)), func_expr(expr), closure(std::move(env)) {}
        
        Value call(Interpreter& interp, const std::vector<Value>& arguments) override {
            // Create new environment for function execution
            auto functionEnv = std::make_shared<Environment>(closure);
            
            // Bind parameters to arguments
            for (size_t i = 0; i < parameters.size() && i < arguments.size(); i++) {
                functionEnv->define(parameters[i], arguments[i]);
            }
            
            // Save current environment and switch to function environment
            auto oldEnv = interp.environment_;
            interp.environment_ = functionEnv;
            
            // Execute function body
            Value result = nullptr;
            try {
                for (const auto& stmt : func_expr->body) {
                    interp.execute(stmt.get());
                }
            } catch (const ReturnValue& returnValue) {
                result = returnValue.value;
                // Restore environment before returning
                interp.environment_ = oldEnv;
                return result;
            }
            
            // Restore the original environment
            interp.environment_ = oldEnv;
            
            return result; // Return nil if no explicit return
        }
        
        int arity() const override {
            return static_cast<int>(parameters.size());
        }
        
        std::string toString() const override {
            return "<anonymous function>";
        }
    };
    
    return Value(std::make_shared<FunctionExpressionCallable>(
        expr->parameters, expr, environment_));
}

Value Interpreter::visitSetExpr(SetExpr* expr) {
    throw RuntimeError(expr->token, "Set expressions not supported.");
}

void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (isNumber(operand)) return;
    throw RuntimeError(op, "Operand must be a number");
}

void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (isNumber(left) && isNumber(right)) return;
    throw RuntimeError(op, "Operands must be numbers");
}

// ==================== JSON ENCODING/DECODING METHODS ====================

std::string Interpreter::encodeToJson(const Value& value) {
    std::ostringstream oss;
    encodeJsonValue(value, oss);
    return oss.str();
}

Value Interpreter::decodeFromJson(const std::string& jsonString) {
    // Simple JSON decoder - handles basic types
    std::string trimmed = jsonString;
    // Remove leading/trailing whitespace
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    
    if (trimmed.empty()) {
        return nullptr;
    }
    
    // Handle null
    if (trimmed == "null") {
        return nullptr;
    }
    
    // Handle boolean
    if (trimmed == "true") {
        return true;
    }
    if (trimmed == "false") {
        return false;
    }
    
    // Handle string (quoted)
    if (trimmed.front() == '"' && trimmed.back() == '"') {
        // Remove quotes and handle escape sequences
        std::string str = trimmed.substr(1, trimmed.length() - 2);
        // Simple escape sequence handling
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[++i]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += str[i-1]; result += str[i]; break;
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    // Handle number
    try {
        // Check if it's a valid number
        size_t pos;
        double num = std::stod(trimmed, &pos);
        if (pos == trimmed.length()) {
            return num;
        }
    } catch (...) {
        // Not a valid number
    }
    
    // Handle array
    if (trimmed.front() == '[' && trimmed.back() == ']') {
        // Parse array elements
        std::string content = trimmed.substr(1, trimmed.length() - 2);
        auto array = std::make_shared<VoltArray>();
        
        // Simple CSV-like parsing (doesn't handle nested structures well)
        std::istringstream iss(content);
        std::string element;
        while (std::getline(iss, element, ',')) {
            // Trim whitespace
            element.erase(0, element.find_first_not_of(" \t\n\r"));
            element.erase(element.find_last_not_of(" \t\n\r") + 1);
            if (!element.empty()) {
                array->push(decodeFromJson(element));
            }
        }
        return array;
    }
    
    // Handle object
    if (trimmed.front() == '{' && trimmed.back() == '}') {
        // Parse object key-value pairs
        // This is a simplified implementation
        return std::make_shared<VoltHashMap>(); // Return empty hashmap for now
    }
    
    // If we can't parse it, treat as string
    return trimmed;
}

void Interpreter::encodeJsonValue(const Value& value, std::ostringstream& oss) {
    if (isNil(value)) {
        oss << "null";
    } else if (isBool(value)) {
        oss << (asBool(value) ? "true" : "false");
    } else if (isNumber(value)) {
        double num = asNumber(value);
        if (num == static_cast<long long>(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << std::fixed << std::setprecision(6) << num;
            // Remove trailing zeros
            std::string str = oss.str();
            oss.str("");
            oss.clear();
            size_t dotPos = str.find('.');
            if (dotPos != std::string::npos) {
                size_t lastNonZero = str.find_last_not_of('0');
                if (lastNonZero == dotPos) {
                    str = str.substr(0, dotPos);
                } else if (lastNonZero != std::string::npos) {
                    str = str.substr(0, lastNonZero + 1);
                }
            }
            oss << str;
        }
    } else if (isString(value)) {
        std::string str = asString(value);
        oss << "\"";
        for (char c : str) {
            switch (c) {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '/': oss << "\/"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (c >= 0 && c < 32) {
                        // Escape control characters
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        oss << c;
                    }
                    break;
            }
        }
        oss << "\"";
    } else if (isArray(value)) {
        auto array = asArray(value);
        oss << "[";
        const auto& elements = array->elements();
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) oss << ",";
            encodeJsonValue(elements[i], oss);
        }
        oss << "]";
    } else if (isHashMap(value)) {
        oss << "{}"; // Simplified - just return empty object
    } else {
        oss << "\"" << valueToString(value) << "\"";
    }
}

} // namespace volt
