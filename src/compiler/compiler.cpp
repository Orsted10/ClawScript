#include "compiler.h"
#include "features/string_pool.h"
#include <iostream>

namespace volt {

Compiler::Compiler() : currentLine_(0), scopeDepth_(0) {}

std::unique_ptr<Chunk> Compiler::compile(const std::vector<StmtPtr>& program) {
    chunk_ = std::make_unique<Chunk>();
    locals_.clear();
    scopeDepth_ = 0;
    
    for (const auto& stmt : program) {
        currentLine_ = stmt->token.line;
        stmt->accept(*this);
    }
    
    emitOp(OpCode::Return);
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

Value Compiler::visitCallExpr(CallExpr* expr) { return nilValue(); }

Value Compiler::visitAssignExpr(AssignExpr* expr) {
    expr->value->accept(*this);
    
    std::string_view name = expr->token.lexeme;
    int arg = resolveLocal(name);
    if (arg != -1) {
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(arg));
    } else {
        emitOp(OpCode::SetGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}

Value Compiler::visitCompoundAssignExpr(CompoundAssignExpr* expr) { return nilValue(); }
Value Compiler::visitUpdateExpr(UpdateExpr* expr) { return nilValue(); }
Value Compiler::visitTernaryExpr(TernaryExpr* expr) { return nilValue(); }
Value Compiler::visitArrayExpr(ArrayExpr* expr) { return nilValue(); }
Value Compiler::visitIndexExpr(IndexExpr* expr) { return nilValue(); }
Value Compiler::visitIndexAssignExpr(IndexAssignExpr* expr) { return nilValue(); }
Value Compiler::visitHashMapExpr(HashMapExpr* expr) { return nilValue(); }
Value Compiler::visitMemberExpr(MemberExpr* expr) { return nilValue(); }
Value Compiler::visitSetExpr(SetExpr* expr) { return nilValue(); }
Value Compiler::visitThisExpr(ThisExpr* expr) { return nilValue(); }
Value Compiler::visitSuperExpr(SuperExpr* expr) { return nilValue(); }
Value Compiler::visitFunctionExpr(FunctionExpr* expr) { return nilValue(); }

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
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
    } else {
        emitOp(OpCode::Nil);
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
void Compiler::visitRunUntilStmt(RunUntilStmt* stmt) {}
void Compiler::visitForStmt(ForStmt* stmt) {
    beginScope();
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
    }

    int loopStart = static_cast<int>(chunk_->size());
    int exitJump = -1;
    if (stmt->condition) {
        stmt->condition->accept(*this);
        exitJump = emitJump(OpCode::JumpIfFalse);
        emitOp(OpCode::Pop); // Condition
    }

    stmt->body->accept(*this);

    if (stmt->increment) {
        stmt->increment->accept(*this);
        emitOp(OpCode::Pop);
    }

    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitOp(OpCode::Pop); // Condition
    }

    endScope();
}
void Compiler::visitFnStmt(FnStmt* stmt) {}
void Compiler::visitReturnStmt(ReturnStmt* stmt) {}
void Compiler::visitBreakStmt(BreakStmt* stmt) {}
void Compiler::visitContinueStmt(ContinueStmt* stmt) {}
void Compiler::visitTryStmt(TryStmt* stmt) {}
void Compiler::visitThrowStmt(ThrowStmt* stmt) {}
void Compiler::visitImportStmt(ImportStmt* stmt) {}
void Compiler::visitClassStmt(ClassStmt* stmt) {}

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
        emitOp(OpCode::Pop);
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

void Compiler::error(Token token, const std::string& message) {
    std::cerr << "[line " << token.line << "] Error: " << message << std::endl;
}

} // namespace volt
