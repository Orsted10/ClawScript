#include "compiler.h"
#include "features/string_pool.h"
#include <iostream>
#include <cmath>
#include <cstdint>

namespace claw {

Compiler::Compiler() : currentLine_(0), scopeDepth_(0), enclosing_(nullptr) {}
Compiler::Compiler(Compiler* enclosing) : currentLine_(0), scopeDepth_(0), enclosing_(enclosing) {}

std::unique_ptr<Chunk> Compiler::compile(const std::vector<StmtPtr>& program) {
    chunk_ = std::make_unique<Chunk>();
    locals_.clear();
    upvalues_.clear();
    scopeDepth_ = 0;
    enclosing_ = nullptr;
    
    for (const auto& stmt : program) {
        currentLine_ = stmt->token.line;
        stmt->accept(*this);
    }
    
    emitOp(OpCode::Return);
    chunk_->setLoopCount(static_cast<int>(chunk_->countOpcode(OpCode::Loop)));
    return std::move(chunk_);
}

// ExprVisitor implementation

Value Compiler::visitLiteralExpr(LiteralExpr* expr) {
    switch (expr->type) {
        case LiteralExpr::Type::Number:
            emitConstant(numberToValue(expr->numberValue));
            break;
        case LiteralExpr::Type::String:
            emitConstant(stringValue(StringPool::intern(expr->stringValue).data()));
            break;
        case LiteralExpr::Type::Bool:
            emitOp(expr->boolValue ? OpCode::True : OpCode::False);
            break;
        case LiteralExpr::Type::Nil:
            emitOp(OpCode::Nil);
            break;
    }
    return nilValue(); // Compiler doesn't return Values during compilation
}

Value Compiler::visitVariableExpr(VariableExpr* expr) {
    std::string_view name = expr->token.lexeme;
    int arg = resolveLocal(name);
    
    if (arg != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(arg));
    } else if ((arg = resolveUpvalue(name)) != -1) {
        emitOp(OpCode::GetUpvalue);
        emitByte(static_cast<uint8_t>(arg));
    } else {
        emitOp(OpCode::GetGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}

Value Compiler::visitBinaryExpr(BinaryExpr* expr) {
    expr->left->accept(*this);
    expr->right->accept(*this);
    
    switch (expr->op.type) {
        case TokenType::Plus:  emitOp(OpCode::Add); break;
        case TokenType::Minus: emitOp(OpCode::Subtract); break;
        case TokenType::Star:  emitOp(OpCode::Multiply); break;
        case TokenType::Slash: emitOp(OpCode::Divide); break;
        case TokenType::BitAnd:       emitOp(OpCode::BitAnd); break;
        case TokenType::BitOr:        emitOp(OpCode::BitOr); break;
        case TokenType::BitXor:       emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeft:    emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRight:   emitOp(OpCode::ShiftRight); break;
        case TokenType::Greater:      emitOp(OpCode::Greater); break;
        case TokenType::GreaterEqual: {
            emitOp(OpCode::Less);
            emitOp(OpCode::Not);
            break;
        }
        case TokenType::Less:         emitOp(OpCode::Less); break;
        case TokenType::LessEqual: {
            emitOp(OpCode::Greater);
            emitOp(OpCode::Not);
            break;
        }
        case TokenType::EqualEqual:   emitOp(OpCode::Equal); break;
        case TokenType::BangEqual: {
            emitOp(OpCode::Equal);
            emitOp(OpCode::Not);
            break;
        }
        default: return nilValue();
    }
    return nilValue();
}

Value Compiler::visitUnaryExpr(UnaryExpr* expr) {
    expr->right->accept(*this);
    
    switch (expr->op.type) {
        case TokenType::Minus: emitOp(OpCode::Negate); break;
        case TokenType::Bang:  emitOp(OpCode::Not); break;
        default: return nilValue();
    }
    return nilValue();
}

Value Compiler::visitLogicalExpr(LogicalExpr* expr) {
    // Basic logical op - could be optimized with short-circuiting jumps
    expr->left->accept(*this);
    expr->right->accept(*this);
    return nilValue();
}

Value Compiler::visitGroupingExpr(GroupingExpr* expr) {
    return expr->expr->accept(*this);
}

Value Compiler::visitCallExpr(CallExpr* expr) {
    if (auto* var = dynamic_cast<VariableExpr*>(expr->callee.get())) {
        if (var->token.lexeme == std::string_view("num") && expr->arguments.size() == 1) {
            if (auto* lit = dynamic_cast<LiteralExpr*>(expr->arguments[0].get())) {
                if (lit->type == LiteralExpr::Type::Number) {
                    expr->arguments[0]->accept(*this);
                    return nilValue();
                }
            }
        }
    }
    expr->callee->accept(*this);
    uint8_t argCount = 0;
    for (const auto& argument : expr->arguments) {
        argument->accept(*this);
        argCount++;
    }
    emitOp(OpCode::Call);
    emitByte(argCount);
    return nilValue();
}

Value Compiler::visitAssignExpr(AssignExpr* expr) {
    expr->value->accept(*this);
    
    std::string_view name = expr->token.lexeme;
    int arg = resolveLocal(name);
    if (arg != -1) {
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(arg));
    } else if ((arg = resolveUpvalue(name)) != -1) {
        emitOp(OpCode::SetUpvalue);
        emitByte(static_cast<uint8_t>(arg));
    } else {
        emitOp(OpCode::SetGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}

Value Compiler::visitCompoundAssignExpr(CompoundAssignExpr* expr) {
    // Load current variable value
    int local = resolveLocal(expr->name);
    int upv = -1;
    if (local != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(local));
    } else if ((upv = resolveUpvalue(expr->name)) != -1) {
        emitOp(OpCode::GetUpvalue);
        emitByte(static_cast<uint8_t>(upv));
    } else {
        emitOp(OpCode::GetGlobal);
        auto sv = StringPool::intern(expr->name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    // Operand
    expr->value->accept(*this);
    // Operation
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    // Store back
    if (local != -1) {
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(local));
    } else if (upv != -1) {
        emitOp(OpCode::SetUpvalue);
        emitByte(static_cast<uint8_t>(upv));
    } else {
        emitOp(OpCode::SetGlobal);
        auto sv = StringPool::intern(expr->name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}
Value Compiler::visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) {
    beginScope();
    std::string tmpObj = std::string("$tmp_o_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpObj);
    int objSlot = resolveLocal(tmpObj);
    emitOp(OpCode::Nil);
    expr->object->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::Pop);
    std::string tmpRhs = std::string("$tmp_rhs_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRhs);
    int rhsSlot = resolveLocal(tmpRhs);
    emitOp(OpCode::Nil);
    expr->value->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::Pop);
    auto svName = StringPool::intern(expr->member);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::EnsurePropertyDefault);
    emitByte(makeConstant(stringValue(svName.data())));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitByte(0); break;
        case TokenType::MinusEqual:       emitByte(1); break;
        case TokenType::StarEqual:        emitByte(2); break;
        case TokenType::SlashEqual:       emitByte(3); break;
        case TokenType::BitAndEqual:      emitByte(4); break;
        case TokenType::BitOrEqual:       emitByte(5); break;
        case TokenType::BitXorEqual:      emitByte(6); break;
        case TokenType::ShiftLeftEqual:   emitByte(7); break;
        case TokenType::ShiftRightEqual:  emitByte(8); break;
        default:                           emitByte(255); break;
    }
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    std::string tmpRes = std::string("$tmp_r_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRes);
    int resSlot = resolveLocal(tmpRes);
    emitOp(OpCode::Nil);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetProperty);
    emitByte(makeConstant(stringValue(svName.data())));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::Pop);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::SetProperty);
    emitByte(makeConstant(stringValue(svName.data())));
    endScope();
    return nilValue();
}
Value Compiler::visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) {
    beginScope();
    // Allocate locals and stash values once
    std::string tmpObj = std::string("$tmp_o_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpObj);
    int objSlot = resolveLocal(tmpObj);
    emitOp(OpCode::Nil);
    expr->object->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::Pop);
    std::string tmpIdx = std::string("$tmp_i_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpIdx);
    int idxSlot = resolveLocal(tmpIdx);
    emitOp(OpCode::Nil);
    expr->index->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::Pop);
    std::string tmpRhs = std::string("$tmp_rhs_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRhs);
    int rhsSlot = resolveLocal(tmpRhs);
    emitOp(OpCode::Nil);
    expr->value->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::Pop);
    // Ensure default for missing hash keys
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::EnsureIndexDefault);
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitByte(0); break;
        case TokenType::MinusEqual:       emitByte(1); break;
        case TokenType::StarEqual:        emitByte(2); break;
        case TokenType::SlashEqual:       emitByte(3); break;
        case TokenType::BitAndEqual:      emitByte(4); break;
        case TokenType::BitOrEqual:       emitByte(5); break;
        case TokenType::BitXorEqual:      emitByte(6); break;
        case TokenType::ShiftLeftEqual:   emitByte(7); break;
        case TokenType::ShiftRightEqual:  emitByte(8); break;
        default:                           emitByte(255); break;
    }
    // Clear the temporary stack triplet [obj, idx, rhs] used by EnsureIndexDefault
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    // Allocate result slot before computing to ensure it resides below eval stack
    std::string tmpRes = std::string("$tmp_r_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRes);
    int resSlot = resolveLocal(tmpRes);
    emitOp(OpCode::Nil);
    // Load current value at index, apply operation with RHS
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetIndex);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::Pop);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::SetIndex);
    endScope();
    return nilValue();
}
Value Compiler::visitUpdateExpr(UpdateExpr* expr) {
    int slot = resolveLocal(expr->name);
    if (slot != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(slot));
        emitOp(OpCode::Constant);
        emitByte(makeConstant(numberToValue(1.0)));
        if (expr->op.type == TokenType::PlusPlus) {
            emitOp(OpCode::Add);
        } else {
            emitOp(OpCode::Subtract);
        }
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(slot));
        return nilValue();
    }
    return nilValue();
}
Value Compiler::visitUpdateMemberExpr(UpdateMemberExpr* expr) { return nilValue(); }
Value Compiler::visitUpdateIndexExpr(UpdateIndexExpr* expr) { return nilValue(); }
Value Compiler::visitTernaryExpr(TernaryExpr* expr) {
    expr->condition->accept(*this);
    int elseJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    expr->thenBranch->accept(*this);
    int endJump = emitJump(OpCode::Jump);
    patchJump(elseJump);
    emitOp(OpCode::Pop);
    expr->elseBranch->accept(*this);
    patchJump(endJump);
    return nilValue();
}
Value Compiler::visitArrayExpr(ArrayExpr* expr) { return nilValue(); }
Value Compiler::visitIndexExpr(IndexExpr* expr) {
    expr->object->accept(*this);
    expr->index->accept(*this);
    emitOp(OpCode::GetIndex);
    return nilValue();
}
Value Compiler::visitIndexAssignExpr(IndexAssignExpr* expr) {
    expr->object->accept(*this);
    expr->index->accept(*this);
    expr->value->accept(*this);
    emitOp(OpCode::SetIndex);
    return nilValue();
}
Value Compiler::visitHashMapExpr(HashMapExpr* expr) { return nilValue(); }
Value Compiler::visitMemberExpr(MemberExpr* expr) {
    expr->object->accept(*this);
    emitOp(OpCode::GetProperty);
    auto sv = StringPool::intern(expr->member);
    emitByte(makeConstant(stringValue(sv.data())));
    return nilValue();
}
Value Compiler::visitSetExpr(SetExpr* expr) {
    expr->object->accept(*this);
    expr->value->accept(*this);
    emitOp(OpCode::SetProperty);
    auto sv = StringPool::intern(expr->member);
    emitByte(makeConstant(stringValue(sv.data())));
    return nilValue();
}
Value Compiler::visitThisExpr(ThisExpr* expr) { return nilValue(); }
Value Compiler::visitSuperExpr(SuperExpr* expr) { return nilValue(); }
Value Compiler::visitFunctionExpr(FunctionExpr* expr) {
    Compiler functionCompiler(this);
    functionCompiler.chunk_ = std::make_unique<Chunk>();
    functionCompiler.beginScope();
    functionCompiler.addLocal("");

    for (const auto& param : expr->parameters) {
        functionCompiler.addLocal(param);
    }

    for (const auto& stmt : expr->body) {
        functionCompiler.currentLine_ = stmt->token.line;
        stmt->accept(functionCompiler);
    }

    functionCompiler.emitOp(OpCode::Nil);
    functionCompiler.emitOp(OpCode::Return);

    auto function = std::make_shared<VMFunction>();
    function->name = "<lambda>";
    function->arity = static_cast<int>(expr->parameters.size());
    function->upvalueCount = static_cast<int>(functionCompiler.upvalues_.size());
    function->chunk = std::move(functionCompiler.chunk_);

    emitOp(OpCode::Closure);
    emitByte(makeConstant(vmFunctionValue(function)));

    for (const auto& upvalue : functionCompiler.upvalues_) {
        emitByte(upvalue.isLocal ? 1 : 0);
        emitByte(upvalue.index);
    }

    return nilValue();
}

// StmtVisitor implementation

void Compiler::visitExprStmt(ExprStmt* stmt) {
    stmt->expr->accept(*this);
    emitOp(OpCode::Pop);
}

void Compiler::visitPrintStmt(PrintStmt* stmt) {
    stmt->expr->accept(*this);
    emitOp(OpCode::Print);
}

void Compiler::visitLetStmt(LetStmt* stmt) {
    std::string_view name = stmt->token.lexeme;
    if (scopeDepth_ > 0) {
        addLocal(name);
        int slot = resolveLocal(name);
        emitOp(OpCode::Nil);
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
            emitOp(OpCode::SetLocal);
            emitByte(static_cast<uint8_t>(slot));
            emitOp(OpCode::Pop);
        }
    } else {
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
        } else {
            emitOp(OpCode::Nil);
        }
        emitOp(OpCode::DefineGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
}

void Compiler::visitBlockStmt(BlockStmt* stmt) {
    beginScope();
    for (const auto& s : stmt->statements) {
        s->accept(*this);
    }
    endScope();
}

void Compiler::visitIfStmt(IfStmt* stmt) {
    stmt->condition->accept(*this);
    int thenJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop); // Pop condition
    
    stmt->thenBranch->accept(*this);
    
    int elseJump = emitJump(OpCode::Jump);
    
    patchJump(thenJump);
    emitOp(OpCode::Pop); // Pop condition
    
    if (stmt->elseBranch) {
        stmt->elseBranch->accept(*this);
    }
    patchJump(elseJump);
}

void Compiler::visitWhileStmt(WhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk_->size());
    stmt->condition->accept(*this);
    
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    
    stmt->body->accept(*this);
    emitLoop(loopStart);
    
    patchJump(exitJump);
    emitOp(OpCode::Pop);
}
void Compiler::visitRunUntilStmt(RunUntilStmt*) {}
void Compiler::visitForStmt(ForStmt* stmt) {
    beginScope();
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
    }
    bool unrolled = false;
    if (stmt->initializer && stmt->condition && stmt->increment) {
        auto initLet = dynamic_cast<LetStmt*>(stmt->initializer.get());
        auto condBin = dynamic_cast<BinaryExpr*>(stmt->condition.get());
        auto incAssign = dynamic_cast<AssignExpr*>(stmt->increment.get());
        auto incUpdate = dynamic_cast<UpdateExpr*>(stmt->increment.get());
        if (initLet && initLet->initializer) {
            auto initLit = dynamic_cast<LiteralExpr*>(initLet->initializer.get());
            if (initLit && initLit->type == LiteralExpr::Type::Number && condBin) {
                auto leftVar = dynamic_cast<VariableExpr*>(condBin->left.get());
                auto rightLit = dynamic_cast<LiteralExpr*>(condBin->right.get());
                if (leftVar && rightLit && rightLit->type == LiteralExpr::Type::Number) {
                    double startVal = initLit->numberValue;
                    double limitVal = rightLit->numberValue;
                    double stepVal = 0.0;
                    bool stepOk = false;
                    if (incAssign) {
                        auto valBin = dynamic_cast<BinaryExpr*>(incAssign->value.get());
                        if (valBin && valBin->op.type == TokenType::Plus) {
                            auto vleft = dynamic_cast<VariableExpr*>(valBin->left.get());
                            auto vright = dynamic_cast<LiteralExpr*>(valBin->right.get());
                            if (vleft && vright && vright->type == LiteralExpr::Type::Number &&
                                vleft->name == leftVar->name) {
                                stepVal = vright->numberValue;
                                stepOk = true;
                            }
                        }
                    } else if (incUpdate && incUpdate->op.type == TokenType::PlusPlus &&
                               incUpdate->name == leftVar->name) {
                        stepVal = 1.0;
                        stepOk = true;
                    }
                    if (stepOk && stepVal > 0.0 && leftVar->name == initLet->name) {
                        int iterations = 0;
                        if (condBin->op.type == TokenType::Less) {
                            iterations = static_cast<int>(std::max(0.0, std::floor((limitVal - startVal) / stepVal)));
                        } else if (condBin->op.type == TokenType::LessEqual) {
                            iterations = static_cast<int>(std::max(0.0, std::floor((limitVal - startVal) / stepVal) + 1.0));
                        }
                        if (iterations > 0 && iterations <= 16) {
                            for (int k = 0; k < iterations; ++k) {
                                stmt->body->accept(*this);
                                stmt->increment->accept(*this);
                                emitOp(OpCode::Pop);
                            }
                            unrolled = true;
                        }
                    }
                }
            }
        }
    }
    if (!unrolled) {
        int loopStart = static_cast<int>(chunk_->size());
        int exitJump = -1;
        if (stmt->condition) {
            stmt->condition->accept(*this);
            exitJump = emitJump(OpCode::JumpIfFalse);
            emitOp(OpCode::Pop);
        }
        stmt->body->accept(*this);
        if (stmt->increment) {
            stmt->increment->accept(*this);
            emitOp(OpCode::Pop);
        }
        emitLoop(loopStart);
        if (exitJump != -1) {
            patchJump(exitJump);
            emitOp(OpCode::Pop);
        }
    }
    endScope();
}
void Compiler::visitFnStmt(FnStmt* stmt) {
    Compiler functionCompiler(this);
    functionCompiler.chunk_ = std::make_unique<Chunk>();
    functionCompiler.beginScope();
    functionCompiler.addLocal(stmt->name);

    for (const auto& param : stmt->parameters) {
        functionCompiler.addLocal(param);
    }

    for (const auto& bodyStmt : stmt->body) {
        functionCompiler.currentLine_ = bodyStmt->token.line;
        bodyStmt->accept(functionCompiler);
    }

    functionCompiler.emitOp(OpCode::Nil);
    functionCompiler.emitOp(OpCode::Return);

    auto function = std::make_shared<VMFunction>();
    function->name = stmt->name;
    function->arity = static_cast<int>(stmt->parameters.size());
    function->upvalueCount = static_cast<int>(functionCompiler.upvalues_.size());
    function->chunk = std::move(functionCompiler.chunk_);

    emitOp(OpCode::Closure);
    emitByte(makeConstant(vmFunctionValue(function)));

    for (const auto& upvalue : functionCompiler.upvalues_) {
        emitByte(upvalue.isLocal ? 1 : 0);
        emitByte(upvalue.index);
    }

    std::string_view name = stmt->token.lexeme;
    if (scopeDepth_ > 0) {
        addLocal(name);
    } else {
        emitOp(OpCode::DefineGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
}
void Compiler::visitReturnStmt(ReturnStmt* stmt) {
    if (stmt->value) {
        stmt->value->accept(*this);
    } else {
        emitOp(OpCode::Nil);
    }
    emitOp(OpCode::Return);
}
void Compiler::visitBreakStmt(BreakStmt*) {}
void Compiler::visitContinueStmt(ContinueStmt*) {}
void Compiler::visitTryStmt(TryStmt*) {}
void Compiler::visitThrowStmt(ThrowStmt*) {}
void Compiler::visitImportStmt(ImportStmt*) {}
void Compiler::visitClassStmt(ClassStmt*) {}
void Compiler::visitSwitchStmt(SwitchStmt*) {}

// Private helpers

void Compiler::emitByte(uint8_t byte) {
    chunk_->write(byte, currentLine_);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitOp(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

void Compiler::emitConstant(Value value) {
    emitOp(OpCode::Constant);
    emitByte(makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int index = chunk_->addConstant(value);
    if (index > 255) {
        error(Token(TokenType::Error, "", currentLine_), "Too many constants in one chunk.");
        return 0;
    }
    return static_cast<uint8_t>(index);
}

int Compiler::emitJump(OpCode instruction) {
    emitOp(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return static_cast<int>(chunk_->size() - 2);
}

void Compiler::patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = static_cast<int>(chunk_->size() - offset - 2);

    if (jump > UINT16_MAX) {
        error(Token(TokenType::Error, "", currentLine_), "Too much code to jump over.");
    }

    const_cast<std::vector<uint8_t>&>(chunk_->code())[offset] = (jump >> 8) & 0xff;
    const_cast<std::vector<uint8_t>&>(chunk_->code())[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emitOp(OpCode::Loop);

    int offset = static_cast<int>(chunk_->size() - loopStart + 2);
    if (offset > UINT16_MAX) error(Token(TokenType::Error, "", currentLine_), "Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::beginScope() {
    scopeDepth_++;
}

void Compiler::endScope() {
    scopeDepth_--;
    
    while (!locals_.empty() && locals_.back().depth > scopeDepth_) {
        if (locals_.back().isCaptured) {
            emitOp(OpCode::CloseUpvalue);
        } else {
            emitOp(OpCode::Pop);
        }
        locals_.pop_back();
    }
}

void Compiler::addLocal(std::string_view name) {
    if (locals_.size() >= 256) {
        error(Token(TokenType::Error, "", currentLine_), "Too many local variables in function.");
        return;
    }

    name = StringPool::intern(name);

    // Check if variable already exists in same scope
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; i--) {
        Local& local = locals_[i];
        if (local.depth != -1 && local.depth < scopeDepth_) break;
        
        if (local.name.data() == name.data()) {
            error(Token(TokenType::Error, "", currentLine_), "Already a variable with this name in this scope.");
        }
    }

    Local local;
    local.name = name;
    local.depth = scopeDepth_;
    local.isCaptured = false;
    locals_.push_back(local);
}

int Compiler::resolveLocal(std::string_view name) {
    name = StringPool::intern(name);
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; i--) {
        if (locals_[i].name.data() == name.data()) {
            return i;
        }
    }
    return -1;
}

int Compiler::resolveUpvalue(std::string_view name) {
    if (!enclosing_) return -1;

    int local = enclosing_->resolveLocal(name);
    if (local != -1) {
        enclosing_->locals_[local].isCaptured = true;
        return addUpvalue(static_cast<uint8_t>(local), true);
    }

    int upvalue = enclosing_->resolveUpvalue(name);
    if (upvalue != -1) {
        return addUpvalue(static_cast<uint8_t>(upvalue), false);
    }

    return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal) {
    for (size_t i = 0; i < upvalues_.size(); i++) {
        if (upvalues_[i].index == index && upvalues_[i].isLocal == isLocal) {
            return static_cast<int>(i);
        }
    }

    if (upvalues_.size() >= 256) {
        error(Token(TokenType::Error, "", currentLine_), "Too many closure variables in function.");
        return 0;
    }

    upvalues_.push_back(Upvalue{index, isLocal});
    return static_cast<int>(upvalues_.size() - 1);
}

void Compiler::error(Token token, const std::string& message) {
    std::cerr << "[line " << token.line << "] Error: " << message << std::endl;
}

} // namespace claw
