#pragma once
#include <stack>
#include <vector>

#include "include/ast/expressions.h"
#include "include/semantic/symbol_table.h"
#include "visitor.h"

class TACInstruction {
public:
    enum class OpCode {
        Label,
        Function,        // name, param_count, global
        StaticVariable,  // name, initializer, global
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
        BitwiseAnd,
        BitwiseXor,
        BitwiseOr,
        LeftShift,
        RightShift,
        Call,
        Param,
        SignExtend,
        ZeroExtend,
        Truncate,
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

    bool operator==(const TACInstruction& other) const;

private:
    OpCode op_;
    std::string dst_;
    std::string lhs_;
    std::string rhs_;
    std::string label_;
};

///////////////////////////////////////////////

class TACVisitor : public Visitor {
public:
    explicit TACVisitor(SymbolTable& symbol_table);
    void Visit(TranslationUnit* translation_unit) override;
    void Visit(ItemList* item_list) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(DeclarationSpecifiers* decl_specs) override;
    void Visit(TypeSpecification* type) override;
    void Visit(Declaration* declaration) override;
    void Visit(Expression* expression) override;
    void Visit(IdExpression* expression) override;
    void Visit(PrimaryExpression* expression) override;
    void Visit(UnaryExpression* expression) override;
    void Visit(BinaryExpression* expression) override;
    void Visit(ConditionalExpression* expression) override;
    void Visit(AssignmentExpression* expression) override;
    void Visit(CastExpression* expression) override;
    void Visit(CompoundStatement* statement) override;
    void Visit(ReturnStatement* statement) override;
    void Visit(ExpressionStatement* statement) override;
    void Visit(SelectionStatement* statement) override;
    void Visit(JumpStatement* statement) override;
    void Visit(WhileStatement* statement) override;
    void Visit(ForStatement* statement) override;
    void Visit(ParameterDeclaration* declaration) override;
    void Visit(ParameterList* list) override;
    void Visit(FunctionCallExpression* expression) override;
    void Visit(ArgumentExpressionList* list) override;
    void Visit(IdentifierDeclarator* declarator) override;
    void Visit(FunctionDeclarator* declarator) override;

    void AddStaticVariables();
    std::vector<std::vector<TACInstruction>> GetTACInstructions() const;
    void PrintTACInstructions(std::ostream& out) const;
    static void PrintTACInstructions(
        std::ostream& out, const std::vector<std::vector<TACInstruction>>& instructions);

private:
    std::string AllocateTemporary(TypeRef type);
    std::string GetTemporaryName();
    std::string GetUniqueLabelId();
    std::string GetTop();

    SymbolTable& symbol_table_;
    std::vector<std::vector<TACInstruction>> instructions_;
    std::stack<std::string> stack_;
    size_t temp_count_ = 0;
    size_t label_id_ = 0;

    void ProcessBinaryOr(BinaryExpression* expression);
    void ProcessBinaryAnd(BinaryExpression* expression);
};

void PrintTACInstructions(std::ostream& out,
                          const std::vector<std::vector<TACInstruction>>& instructions);