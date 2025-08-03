#pragma once
#include <fstream>
#include <vector>

#include "visitor.h"

class TACInstruction {
public:
    enum class OpCode {
        Label,
        Global,
        Return,
        Assign,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Not,
        Plus,
        Minus,
        BinaryNot,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        If,
        IfFalse,
        GoTo,
        EnterScope,
        ExitScope,
    };

    explicit TACInstruction(OpCode op, const std::string& dst, const std::string& lhs,
                            const std::string& rhs);
    explicit TACInstruction(OpCode op, const std::string& dst, const std::string& lhs);
    explicit TACInstruction(OpCode op, const std::string& label);
    explicit TACInstruction(OpCode op);

    std::string ToString() const;
    OpCode GetOp() const;

    const std::string& GetDst() const;
    const std::string& GetLhs() const;
    const std::string& GetRhs() const;
    const std::string& GetLabel() const;

private:
    OpCode op_;
    std::string dst_;
    std::string lhs_;
    std::string rhs_;
    std::string label_;
};

class TACVisitor : public Visitor {
public:
    explicit TACVisitor();
    virtual void Visit(TranslationUnit* translation_unit) override;
    virtual void Visit(ItemList* item_list) override;
    virtual void Visit(FunctionDefinition* function) override;
    virtual void Visit(TypeSpecification* type) override;
    virtual void Visit(Declarator* declarator) override;
    virtual void Visit(InitDeclarator* declarator) override;
    virtual void Visit(Declaration* declaration) override;
    virtual void Visit(IdExpression* expression) override;
    virtual void Visit(PrimaryExpression* expression) override;
    virtual void Visit(UnaryExpression* expression) override;
    virtual void Visit(BinaryExpression* expression) override;
    virtual void Visit(ConditionalExpression* expression) override;
    virtual void Visit(AssignmentExpression* expression) override;
    virtual void Visit(CompoundStatement* statement) override;
    virtual void Visit(ReturnStatement* statement) override;
    virtual void Visit(ExpressionStatement* statement) override;
    virtual void Visit(SelectionStatement* statement) override;

    std::vector<TACInstruction> GetTACInstructions() const;
    void PrintTACInstructions(std::ostream& out) const;

private:
    std::string GetTemporaryName();
    std::string GetUniqueLabelId();
    std::string GetTop();

    std::vector<TACInstruction> instructions_;
    std::stack<std::string> stack_;
    size_t temp_count_ = 0;
    size_t label_id_ = 0;

    void ProcessBinaryOr(BinaryExpression* expression);
    void ProcessBinaryAnd(BinaryExpression* expression);
};