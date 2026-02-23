#pragma once
#include "parser/ast.h"
#include "parser/stmt.h"
#include "vm/chunk.h"
#include <vector>

namespace claw {

/**
 * @brief Compiles AST nodes into Bytecode Chunks
 */
class Compiler : public ExprVisitor, public StmtVisitor {
public:
    Compiler();
    explicit Compiler(Compiler* enclosing);
    ~Compiler() = default;

    std::unique_ptr<Chunk> compile(const std::vector<StmtPtr>& program);

    // ExprVisitor implementation
    Value visitLiteralExpr(LiteralExpr* expr) override;
    Value visitVariableExpr(VariableExpr* expr) override;
    Value visitUnaryExpr(UnaryExpr* expr) override;
    Value visitBinaryExpr(BinaryExpr* expr) override;
    Value visitLogicalExpr(LogicalExpr* expr) override;
    Value visitGroupingExpr(GroupingExpr* expr) override;
    Value visitCallExpr(CallExpr* expr) override;
    Value visitAssignExpr(AssignExpr* expr) override;
    Value visitCompoundAssignExpr(CompoundAssignExpr* expr) override;
    Value visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) override;
    Value visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) override;
    Value visitUpdateExpr(UpdateExpr* expr) override;
    Value visitUpdateMemberExpr(UpdateMemberExpr* expr) override;
    Value visitUpdateIndexExpr(UpdateIndexExpr* expr) override;
    Value visitTernaryExpr(TernaryExpr* expr) override;
    Value visitArrayExpr(ArrayExpr* expr) override;
    Value visitIndexExpr(IndexExpr* expr) override;
    Value visitIndexAssignExpr(IndexAssignExpr* expr) override;
    Value visitHashMapExpr(HashMapExpr* expr) override;
    Value visitMemberExpr(MemberExpr* expr) override;
    Value visitSetExpr(SetExpr* expr) override;
    Value visitThisExpr(ThisExpr* expr) override;
    Value visitSuperExpr(SuperExpr* expr) override;
    Value visitFunctionExpr(FunctionExpr* expr) override;

    // StmtVisitor implementation
    void visitExprStmt(ExprStmt* stmt) override;
    void visitPrintStmt(PrintStmt* stmt) override;
    void visitLetStmt(LetStmt* stmt) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitRunUntilStmt(RunUntilStmt* stmt) override;
    void visitForStmt(ForStmt* stmt) override;
    void visitFnStmt(FnStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;
    void visitBreakStmt(BreakStmt* stmt) override;
    void visitContinueStmt(ContinueStmt* stmt) override;
    void visitTryStmt(TryStmt* stmt) override;
    void visitThrowStmt(ThrowStmt* stmt) override;
    void visitImportStmt(ImportStmt* stmt) override;
    void visitClassStmt(ClassStmt* stmt) override;
    void visitSwitchStmt(SwitchStmt* stmt) override;

private:
    struct Local {
        std::string_view name;
        int depth;
        bool isCaptured;
    };
    struct Upvalue {
        uint8_t index;
        bool isLocal;
    };

    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitOp(OpCode op);
    void emitConstant(Value value);
    uint8_t makeConstant(Value value);
    
    int emitJump(OpCode instruction);
    void patchJump(int offset);
    void emitLoop(int loopStart);

    void beginScope();
    void endScope();
    void addLocal(std::string_view name);
    int resolveLocal(std::string_view name);
    int resolveUpvalue(std::string_view name);
    int addUpvalue(uint8_t index, bool isLocal);

    void error(Token token, const std::string& message);

    std::unique_ptr<Chunk> chunk_;
    int currentLine_;
    
    std::vector<Local> locals_;
    std::vector<Upvalue> upvalues_;
    int scopeDepth_;
    Compiler* enclosing_;
};

} // namespace claw
