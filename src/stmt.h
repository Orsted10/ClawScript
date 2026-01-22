#pragma once
#include "ast.h"
#include <vector>
#include <memory>
#include <string>

namespace volt {

// Forward declaration
struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

// Base statement node
struct Stmt {
    virtual ~Stmt() = default;
};

// Expression statement: expr;
struct ExprStmt : Stmt {
    ExprPtr expr;
    
    explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
};

// Print statement: print expr;
struct PrintStmt : Stmt {
    ExprPtr expr;
    
    explicit PrintStmt(ExprPtr e) : expr(std::move(e)) {}
};

// Variable declaration: let name = expr;
struct LetStmt : Stmt {
    std::string name;
    ExprPtr initializer;
    
    LetStmt(std::string n, ExprPtr init)
        : name(std::move(n)), initializer(std::move(init)) {}
};

// Block statement: { stmts... }
struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    
    explicit BlockStmt(std::vector<StmtPtr> stmts)
        : statements(std::move(stmts)) {}
};

// If statement: if (condition) thenBranch [else elseBranch]
struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;  // can be null
    
    IfStmt(ExprPtr cond, StmtPtr thenB, StmtPtr elseB = nullptr)
        : condition(std::move(cond)), 
          thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}
};

// While statement: while (condition) body
struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    
    WhileStmt(ExprPtr cond, StmtPtr b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// For statement: for (init; condition; increment) body
struct ForStmt : Stmt {
    StmtPtr initializer;  // can be null
    ExprPtr condition;     // can be null
    ExprPtr increment;     // can be null
    StmtPtr body;
    
    ForStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr b)
        : initializer(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(incr)),
          body(std::move(b)) {}
};

// Function declaration: fn name(params...) { body }
struct FnStmt : Stmt {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<StmtPtr> body;
    
    FnStmt(std::string n, 
           std::vector<std::string> params,
           std::vector<StmtPtr> b)
        : name(std::move(n)), 
          parameters(std::move(params)),
          body(std::move(b)) {}
};

// Return statement: return expr;
struct ReturnStmt : Stmt {
    ExprPtr value;  // can be null (just "return;")
    
    explicit ReturnStmt(ExprPtr v) : value(std::move(v)) {}
};

} // namespace volt
