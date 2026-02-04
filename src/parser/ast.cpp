#include "ast.h"
#include "stmt.h"
#include <sstream>

namespace volt {

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
    
    if (auto* update = dynamic_cast<UpdateExpr*>(expr)) {
        std::string op = std::string(update->op.lexeme);
        if (update->prefix) {
            return "(" + op + " " + update->name + ")";
        }
        return "(" + update->name + " " + op + ")";
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

} // namespace volt
