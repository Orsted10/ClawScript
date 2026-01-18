#pragma once
#include <memory>
#include <vector>
#include <string>
#include "token.h"

namespace volt {

// Forward declarations
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

// Base expression node
struct Expr {
    virtual ~Expr() = default;
};

// Literal: 42, 3.14, "hello", true, false, nil
struct LiteralExpr : Expr {
    enum class Type { Number, String, Bool, Nil };
    
    Type type;
    double numberValue;
    std::string stringValue;
    bool boolValue;
    
    explicit LiteralExpr(double value)
        : type(Type::Number), numberValue(value), boolValue(false) {}
    
    explicit LiteralExpr(const std::string& value)
        : type(Type::String), numberValue(0.0), stringValue(value), boolValue(false) {}
    
    explicit LiteralExpr(bool value)
        : type(Type::Bool), numberValue(0.0), boolValue(value) {}
    
    static ExprPtr nil() {
        auto expr = std::make_unique<LiteralExpr>(0.0);
        expr->type = Type::Nil;
        return expr;
    }
};

// Variable: x, myVar
struct VariableExpr : Expr {
    std::string name;
    
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

// Unary: -x, !flag
struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;
    
    UnaryExpr(Token o, ExprPtr r)
        : op(o), right(std::move(r)) {}
};

// Binary: 1 + 2, x * y, a == b
struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// Logical: a && b, x || y
struct LogicalExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    
    LogicalExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// Grouping: (expr)
struct GroupingExpr : Expr {
    ExprPtr expr;
    
    explicit GroupingExpr(ExprPtr e) : expr(std::move(e)) {}
};

// Call: foo(a, b, c)
struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    
    CallExpr(ExprPtr c, std::vector<ExprPtr> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
};

// Assignment: x = 10
struct AssignExpr : Expr {
    std::string name;
    ExprPtr value;
    
    AssignExpr(std::string n, ExprPtr v)
        : name(std::move(n)), value(std::move(v)) {}
};

// AST Pretty Printer
std::string printAST(Expr* expr);

} // namespace volt
