#pragma once
#include <memory>
#include <string>
#include <vector>
#include "token.h"

namespace volt {

// Forward declarations
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

// Base expression node
struct Expr {
    Token token; // Representative token for error reporting
    explicit Expr(Token tok) : token(tok) {}
    virtual ~Expr() = default;
};

// Literal: 42, 3.14, "hello", true, false, nil
struct LiteralExpr : Expr {
    enum class Type { Number, String, Bool, Nil };
    
    Type type;
    double numberValue;
    std::string stringValue;
    bool boolValue;
    
    LiteralExpr(Token tok, double value)
        : Expr(tok), type(Type::Number), numberValue(value), boolValue(false) {}
    
    LiteralExpr(Token tok, const std::string& value)
        : Expr(tok), type(Type::String), numberValue(0.0), stringValue(value), boolValue(false) {}
    
    LiteralExpr(Token tok, bool value)
        : Expr(tok), type(Type::Bool), numberValue(0.0), boolValue(value) {}
    
    static ExprPtr nil(Token tok) {
        auto expr = std::make_unique<LiteralExpr>(tok, 0.0);
        expr->type = Type::Nil;
        return expr;
    }
};

// Variable: x, myVar
struct VariableExpr : Expr {
    std::string name;
    VariableExpr(Token tok, std::string n) : Expr(tok), name(std::move(n)) {}
};

// Unary: -x, !flag
struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;
    UnaryExpr(Token o, ExprPtr r)
        : Expr(o), op(o), right(std::move(r)) {}
};

// Binary: 1 + 2, x * y, a == b
struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : Expr(o), left(std::move(l)), op(o), right(std::move(r)) {}
};

// Logical: a && b, x || y
struct LogicalExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    LogicalExpr(ExprPtr l, Token o, ExprPtr r)
        : Expr(o), left(std::move(l)), op(o), right(std::move(r)) {}
};

// Grouping: (expr)
struct GroupingExpr : Expr {
    ExprPtr expr;
    GroupingExpr(Token tok, ExprPtr e) : Expr(tok), expr(std::move(e)) {}
};

// Call: foo(a, b, c)
struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    CallExpr(Token paren, ExprPtr c, std::vector<ExprPtr> args)
        : Expr(paren), callee(std::move(c)), arguments(std::move(args)) {}
};

// Assignment: x = 10
struct AssignExpr : Expr {
    std::string name;
    ExprPtr value;
    AssignExpr(Token nameTok, ExprPtr v)
        : Expr(nameTok), name(std::string(nameTok.lexeme)), value(std::move(v)) {}
};

// Compound Assignment: x += 10, x -= 5, etc.
struct CompoundAssignExpr : Expr {
    std::string name;
    Token op;
    ExprPtr value;
    CompoundAssignExpr(Token nameTok, Token o, ExprPtr v)
        : Expr(o), name(std::string(nameTok.lexeme)), op(o), value(std::move(v)) {}
};

// Update Expression: ++x, x++, --x, x--
struct UpdateExpr : Expr {
    std::string name;
    Token op;
    bool prefix; // true for ++x, false for x++
    UpdateExpr(Token nameTok, Token o, bool pre)
        : Expr(o), name(std::string(nameTok.lexeme)), op(o), prefix(pre) {}
};

// Ternary: condition ? thenExpr : elseExpr
struct TernaryExpr : Expr {
    ExprPtr condition;
    ExprPtr thenBranch;
    ExprPtr elseBranch;
    TernaryExpr(Token quest, ExprPtr cond, ExprPtr then_, ExprPtr else_)
        : Expr(quest),
          condition(std::move(cond)),
          thenBranch(std::move(then_)),
          elseBranch(std::move(else_)) {}
};

// ========================================
// ARRAY EXPRESSIONS - NEW!
// ========================================

// Array Literal: [1, 2, 3, "hello"]
struct ArrayExpr : Expr {
    std::vector<ExprPtr> elements;
    ArrayExpr(Token bracket, std::vector<ExprPtr> elems)
        : Expr(bracket), elements(std::move(elems)) {}
};

// Array Index Access: arr[0], matrix[i][j]
struct IndexExpr : Expr {
    ExprPtr object;  // The array being indexed
    ExprPtr index;   // The index expression
    
    IndexExpr(Token bracket, ExprPtr obj, ExprPtr idx)
        : Expr(bracket), object(std::move(obj)), index(std::move(idx)) {}
};

// Array Index Assignment: arr[0] = 42
struct IndexAssignExpr : Expr {
    ExprPtr object;  // The array being indexed
    ExprPtr index;   // The index expression
    ExprPtr value;   // The value to assign
    
    IndexAssignExpr(Token bracket, ExprPtr obj, ExprPtr idx, ExprPtr val)
        : Expr(bracket), object(std::move(obj)), index(std::move(idx)), value(std::move(val)) {}
};

// Member Access: array.length, array.push
struct MemberExpr : Expr {
    ExprPtr object;      // The object (array, etc.)
    std::string member;  // The member name (length, push, etc.)
    
    MemberExpr(Token name, ExprPtr obj, std::string mem)
        : Expr(name), object(std::move(obj)), member(std::move(mem)) {}
};

// AST Pretty Printer
std::string printAST(Expr* expr);

} // namespace volt
