#include "ast.h"
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
    
    return "?";
}

} // namespace volt
