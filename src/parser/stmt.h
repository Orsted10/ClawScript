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
    Token token; // Representative token for errors
    explicit Stmt(Token tok) : token(tok) {}
    virtual ~Stmt() = default;
};

// Expression statement: expr;
struct ExprStmt : Stmt {
    ExprPtr expr;
    
    ExprStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
};

// Print statement: print expr;
struct PrintStmt : Stmt {
    ExprPtr expr;
    
    PrintStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
};

// Variable declaration: let name = expr;
struct LetStmt : Stmt {
    std::string name;
    ExprPtr initializer;
    
    LetStmt(Token nameTok, ExprPtr init)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)), initializer(std::move(init)) {}
};

// Block statement: { stmts... }
struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    
    BlockStmt(Token brace, std::vector<StmtPtr> stmts)
        : Stmt(brace), statements(std::move(stmts)) {}
};

// If statement: if (condition) thenBranch [else elseBranch]
struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;  // can be null
    
    IfStmt(Token ifTok, ExprPtr cond, StmtPtr thenB, StmtPtr elseB = nullptr)
        : Stmt(ifTok), condition(std::move(cond)), 
          thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}
};

// While statement: while (condition) body
struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    
    WhileStmt(Token whileTok, ExprPtr cond, StmtPtr b)
        : Stmt(whileTok), condition(std::move(cond)), body(std::move(b)) {}
};

// For statement: for (init; condition; increment) body
struct ForStmt : Stmt {
    StmtPtr initializer;  // can be null
    ExprPtr condition;     // can be null
    ExprPtr increment;     // can be null
    StmtPtr body;
    
    ForStmt(Token forTok, StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr b)
        : Stmt(forTok),
          initializer(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(incr)),
          body(std::move(b)) {}
};

// Function declaration: fn name(params...) { body }
struct FnStmt : Stmt {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<StmtPtr> body;
    
    FnStmt(Token nameTok, 
           std::vector<std::string> params,
           std::vector<StmtPtr> b)
        : Stmt(nameTok),
          name(std::string(nameTok.lexeme)), 
          parameters(std::move(params)),
          body(std::move(b)) {}
};

// Return statement: return expr;
struct ReturnStmt : Stmt {
    ExprPtr value;  // can be null (just "return;")
    
    ReturnStmt(Token returnTok, ExprPtr v) : Stmt(returnTok), value(std::move(v)) {}
};

// Break statement: break;
struct BreakStmt : Stmt {
    explicit BreakStmt(Token tok) : Stmt(tok) {}
};

// Continue statement: continue;
struct ContinueStmt : Stmt {
    explicit ContinueStmt(Token tok) : Stmt(tok) {}
};

} // namespace volt
