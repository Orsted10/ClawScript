#pragma once
#include "ast.h"
#include <vector>
#include <memory>
#include <string>

namespace claw {

// Forward declaration
struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

// Forward declarations for Visitor
struct ExprStmt;
struct PrintStmt;
struct LetStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct RunUntilStmt;
struct ForStmt;
struct FnStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryStmt;
struct ThrowStmt;
struct ImportStmt;
struct ClassStmt;
struct SwitchStmt;

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visitExprStmt(ExprStmt* stmt) = 0;
    virtual void visitPrintStmt(PrintStmt* stmt) = 0;
    virtual void visitLetStmt(LetStmt* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitRunUntilStmt(RunUntilStmt* stmt) = 0;
    virtual void visitForStmt(ForStmt* stmt) = 0;
    virtual void visitFnStmt(FnStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void visitBreakStmt(BreakStmt* stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void visitTryStmt(TryStmt* stmt) = 0;
    virtual void visitThrowStmt(ThrowStmt* stmt) = 0;
    virtual void visitImportStmt(ImportStmt* stmt) = 0;
    virtual void visitClassStmt(ClassStmt* stmt) = 0;
    virtual void visitSwitchStmt(SwitchStmt* stmt) = 0;
};

// Base statement node
struct Stmt {
    Token token; // Representative token for errors
    explicit Stmt(Token tok) : token(tok) {}
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

// Expression statement: expr;
struct ExprStmt : Stmt {
    ExprPtr expr;
    
    ExprStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
    void accept(StmtVisitor& visitor) override;
};

// Print statement: print expr;
struct PrintStmt : Stmt {
    ExprPtr expr;
    
    PrintStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
    void accept(StmtVisitor& visitor) override;
};

// Variable declaration: let name = expr;
struct LetStmt : Stmt {
    std::string name;
    ExprPtr initializer;
    
    LetStmt(Token nameTok, ExprPtr init)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)), initializer(std::move(init)) {}
    void accept(StmtVisitor& visitor) override;
};

// Block statement: { stmts... }
struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    
    BlockStmt(Token brace, std::vector<StmtPtr> stmts)
        : Stmt(brace), statements(std::move(stmts)) {}
    void accept(StmtVisitor& visitor) override;
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
    void accept(StmtVisitor& visitor) override;
};

// While statement: while (condition) body
struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    
    WhileStmt(Token whileTok, ExprPtr cond, StmtPtr b)
        : Stmt(whileTok), condition(std::move(cond)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

// Run-Until statement: run { body } until (condition);
// Executes body at least once, then continues until condition becomes true
struct RunUntilStmt : Stmt {
    StmtPtr body;
    ExprPtr condition;
    
    RunUntilStmt(Token runTok, StmtPtr b, ExprPtr cond)
        : Stmt(runTok), body(std::move(b)), condition(std::move(cond)) {}
    void accept(StmtVisitor& visitor) override;
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
    void accept(StmtVisitor& visitor) override;
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
    void accept(StmtVisitor& visitor) override;
};

// Return statement: return expr;
struct ReturnStmt : Stmt {
    ExprPtr value;  // can be null (just "return;")
    
    ReturnStmt(Token returnTok, ExprPtr v) : Stmt(returnTok), value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override;
};

// Break statement: break;
struct BreakStmt : Stmt {
    explicit BreakStmt(Token tok) : Stmt(tok) {}
    void accept(StmtVisitor& visitor) override;
};

// Continue statement: continue;
struct ContinueStmt : Stmt {
    explicit ContinueStmt(Token tok) : Stmt(tok) {}
    void accept(StmtVisitor& visitor) override;
};

// Function Expression: fun(params) { body }  // Added!
struct FunctionExpr : Expr {
    std::vector<std::string> parameters;
    std::vector<StmtPtr> body;
    
    FunctionExpr(Token keyword, 
                 std::vector<std::string> params,
                 std::vector<StmtPtr> b)
        : Expr(keyword),
          parameters(std::move(params)),
          body(std::move(b)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Try statement: try { body } catch (error) { handler }
struct TryStmt : Stmt {
    StmtPtr tryBody;
    std::string exceptionVar;
    StmtPtr catchBody;
    
    TryStmt(Token tryTok, StmtPtr tryB, std::string exVar, StmtPtr catchB)
        : Stmt(tryTok), tryBody(std::move(tryB)), exceptionVar(std::move(exVar)), catchBody(std::move(catchB)) {}
    void accept(StmtVisitor& visitor) override;
};

// Throw statement: throw expr;
struct ThrowStmt : Stmt {
    ExprPtr expression;
    
    ThrowStmt(Token throwTok, ExprPtr expr) : Stmt(throwTok), expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) override;
};

// Import statement: import { a, b } from "module";
struct ImportStmt : Stmt {
    std::vector<std::string> imports;
    std::string modulePath;
    
    ImportStmt(Token importTok, std::vector<std::string> imps, std::string path)
        : Stmt(importTok), imports(std::move(imps)), modulePath(std::move(path)) {}
    void accept(StmtVisitor& visitor) override;
};

// Class statement: class Name [< Super] { methods... }
struct ClassStmt : Stmt {
    std::string name;
    ExprPtr superclass; // Optional
    std::vector<std::unique_ptr<FnStmt>> methods;
    
    ClassStmt(Token nameTok, ExprPtr super, std::vector<std::unique_ptr<FnStmt>> m)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)), superclass(std::move(super)), methods(std::move(m)) {}
    void accept(StmtVisitor& visitor) override;
};

// Switch statement: switch (expr) { case v: ...; default: ...; }
struct SwitchStmt : Stmt {
    struct Case {
        bool isDefault;
        ExprPtr match; // null for default
        std::vector<StmtPtr> body;
    };
    ExprPtr expression;
    std::vector<Case> cases;
    SwitchStmt(Token switchTok, ExprPtr expr, std::vector<Case> cs)
        : Stmt(switchTok), expression(std::move(expr)), cases(std::move(cs)) {}
    void accept(StmtVisitor& visitor) override;
};

} // namespace claw
