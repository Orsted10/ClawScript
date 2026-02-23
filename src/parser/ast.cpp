#include "ast.h"
#include "stmt.h"
#include <sstream>

namespace claw {

std::string printAST(Expr* expr) {
    if (auto* lit = dynamic_cast<LiteralExpr*>(expr)) {
        switch (lit->type) {
            case LiteralExpr::Type::Number:
                return std::to_string(lit->numberValue);
            case LiteralExpr::Type::String:
                return "\"" + lit->stringValue + "\"";
            case LiteralExpr::Type::Bool:
                return lit->boolValue ? "true" : "false";
            case LiteralExpr::Type::Nil:
                return "nil";
        }
    }
    
    if (auto* var = dynamic_cast<VariableExpr*>(expr)) {
        return var->name;
    }
    
    if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(" << unary->op.lexeme << " " << printAST(unary->right.get()) << ")";
        return oss.str();
    }
    
    if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(" << bin->op.lexeme << " " 
            << printAST(bin->left.get()) << " " 
            << printAST(bin->right.get()) << ")";
        return oss.str();
    }
    
    if (auto* logical = dynamic_cast<LogicalExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(" << logical->op.lexeme << " " 
            << printAST(logical->left.get()) << " " 
            << printAST(logical->right.get()) << ")";
        return oss.str();
    }
    
    if (auto* group = dynamic_cast<GroupingExpr*>(expr)) {
        return "(group " + printAST(group->expr.get()) + ")";
    }
    
    if (auto* call = dynamic_cast<CallExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(call " << printAST(call->callee.get());
        for (auto& arg : call->arguments) {
            oss << " " << printAST(arg.get());
        }
        oss << ")";
        return oss.str();
    }
    
    if (auto* assign = dynamic_cast<AssignExpr*>(expr)) {
        return "(= " + assign->name + " " + printAST(assign->value.get()) + ")";
    }
    
    if (auto* compound = dynamic_cast<CompoundAssignExpr*>(expr)) {
        return "(" + std::string(compound->op.lexeme) + " " + compound->name + " " + printAST(compound->value.get()) + ")";
    }
    
    if (auto* compoundM = dynamic_cast<CompoundMemberAssignExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(" << compoundM->op.lexeme << " ." << compoundM->member << " " << printAST(compoundM->value.get()) << ")";
        return oss.str();
    }
    
    if (auto* compoundI = dynamic_cast<CompoundIndexAssignExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(" << compoundI->op.lexeme << " [index] " << printAST(compoundI->value.get()) << ")";
        return oss.str();
    }
    
    if (auto* update = dynamic_cast<UpdateExpr*>(expr)) {
        std::string op = std::string(update->op.lexeme);
        if (update->prefix) {
            return "(" + op + " " + update->name + ")";
        }
        return "(" + update->name + " " + op + ")";
    }
    
    if (auto* updateM = dynamic_cast<UpdateMemberExpr*>(expr)) {
        std::string op = std::string(updateM->op.lexeme);
        if (updateM->prefix) {
            return "(" + op + " ." + updateM->member + ")";
        }
        return "(. " + updateM->member + " " + op + ")";
    }
    
    if (auto* updateI = dynamic_cast<UpdateIndexExpr*>(expr)) {
        std::string op = std::string(updateI->op.lexeme);
        if (updateI->prefix) {
            return "(" + op + " [index])";
        }
        return "([index] " + op + ")";
    }

 
    
    if (auto* ternary = dynamic_cast<TernaryExpr*>(expr)) {
        return "(?: " + printAST(ternary->condition.get()) + " " + 
               printAST(ternary->thenBranch.get()) + " " + 
               printAST(ternary->elseBranch.get()) + ")";
    }
    
    // ========================================
    // ARRAY EXPRESSIONS - Added!
    // ========================================
    
    if (auto* array = dynamic_cast<ArrayExpr*>(expr)) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < array->elements.size(); i++) {
            if (i > 0) oss << ", ";
            oss << printAST(array->elements[i].get());
        }
        oss << "]";
        return oss.str();
    }
    
    if (auto* index = dynamic_cast<IndexExpr*>(expr)) {
        return printAST(index->object.get()) + "[" + printAST(index->index.get()) + "]";
    }
    
    if (auto* indexAssign = dynamic_cast<IndexAssignExpr*>(expr)) {
        return "([]= " + printAST(indexAssign->object.get()) + " " + 
               printAST(indexAssign->index.get()) + " " + 
               printAST(indexAssign->value.get()) + ")";
    }
    
    if (auto* member = dynamic_cast<MemberExpr*>(expr)) {
        return printAST(member->object.get()) + "." + member->member;
    }
    
    // Hash map expression printing - Added!
    if (auto* hashMap = dynamic_cast<HashMapExpr*>(expr)) {
        std::ostringstream oss;
        oss << "{hashmap " << hashMap->keyValuePairs.size() << " pairs}";
        return oss.str();
    }
    
    // Function expression printing - Added!
    if (auto* funcExpr = dynamic_cast<FunctionExpr*>(expr)) {
        std::ostringstream oss;
        oss << "(function " << funcExpr->parameters.size() << " params)";
        return oss.str();
    }
    
    return "?";
}

// Visitor Pattern Implementations
Value LiteralExpr::accept(ExprVisitor& visitor) { return visitor.visitLiteralExpr(this); }
Value VariableExpr::accept(ExprVisitor& visitor) { return visitor.visitVariableExpr(this); }
Value UnaryExpr::accept(ExprVisitor& visitor) { return visitor.visitUnaryExpr(this); }
Value BinaryExpr::accept(ExprVisitor& visitor) { return visitor.visitBinaryExpr(this); }
Value LogicalExpr::accept(ExprVisitor& visitor) { return visitor.visitLogicalExpr(this); }
Value GroupingExpr::accept(ExprVisitor& visitor) { return visitor.visitGroupingExpr(this); }
Value CallExpr::accept(ExprVisitor& visitor) { return visitor.visitCallExpr(this); }
Value AssignExpr::accept(ExprVisitor& visitor) { return visitor.visitAssignExpr(this); }
Value CompoundAssignExpr::accept(ExprVisitor& visitor) { return visitor.visitCompoundAssignExpr(this); }
Value CompoundMemberAssignExpr::accept(ExprVisitor& visitor) { return visitor.visitCompoundMemberAssignExpr(this); }
Value CompoundIndexAssignExpr::accept(ExprVisitor& visitor) { return visitor.visitCompoundIndexAssignExpr(this); }
Value UpdateExpr::accept(ExprVisitor& visitor) { return visitor.visitUpdateExpr(this); }
Value TernaryExpr::accept(ExprVisitor& visitor) { return visitor.visitTernaryExpr(this); }
Value ArrayExpr::accept(ExprVisitor& visitor) { return visitor.visitArrayExpr(this); }
Value IndexExpr::accept(ExprVisitor& visitor) { return visitor.visitIndexExpr(this); }
Value IndexAssignExpr::accept(ExprVisitor& visitor) { return visitor.visitIndexAssignExpr(this); }
Value HashMapExpr::accept(ExprVisitor& visitor) { return visitor.visitHashMapExpr(this); }
Value MemberExpr::accept(ExprVisitor& visitor) { return visitor.visitMemberExpr(this); }
Value SetExpr::accept(ExprVisitor& visitor) { return visitor.visitSetExpr(this); }
Value ThisExpr::accept(ExprVisitor& visitor) { return visitor.visitThisExpr(this); }
Value SuperExpr::accept(ExprVisitor& visitor) { return visitor.visitSuperExpr(this); }
Value FunctionExpr::accept(ExprVisitor& visitor) { return visitor.visitFunctionExpr(this); }
Value UpdateMemberExpr::accept(ExprVisitor& visitor) { return visitor.visitUpdateMemberExpr(this); }
Value UpdateIndexExpr::accept(ExprVisitor& visitor) { return visitor.visitUpdateIndexExpr(this); }

void ExprStmt::accept(StmtVisitor& visitor) { visitor.visitExprStmt(this); }
void PrintStmt::accept(StmtVisitor& visitor) { visitor.visitPrintStmt(this); }
void LetStmt::accept(StmtVisitor& visitor) { visitor.visitLetStmt(this); }
void BlockStmt::accept(StmtVisitor& visitor) { visitor.visitBlockStmt(this); }
void IfStmt::accept(StmtVisitor& visitor) { visitor.visitIfStmt(this); }
void WhileStmt::accept(StmtVisitor& visitor) { visitor.visitWhileStmt(this); }
void RunUntilStmt::accept(StmtVisitor& visitor) { visitor.visitRunUntilStmt(this); }
void ForStmt::accept(StmtVisitor& visitor) { visitor.visitForStmt(this); }
void FnStmt::accept(StmtVisitor& visitor) { visitor.visitFnStmt(this); }
void ReturnStmt::accept(StmtVisitor& visitor) { visitor.visitReturnStmt(this); }
void BreakStmt::accept(StmtVisitor& visitor) { visitor.visitBreakStmt(this); }
void ContinueStmt::accept(StmtVisitor& visitor) { visitor.visitContinueStmt(this); }
void TryStmt::accept(StmtVisitor& visitor) { visitor.visitTryStmt(this); }
void ThrowStmt::accept(StmtVisitor& visitor) { visitor.visitThrowStmt(this); }
void ImportStmt::accept(StmtVisitor& visitor) { visitor.visitImportStmt(this); }
void ClassStmt::accept(StmtVisitor& visitor) { visitor.visitClassStmt(this); }
void SwitchStmt::accept(StmtVisitor& visitor) { visitor.visitSwitchStmt(this); }

} // namespace claw
