#pragma once
#include "include/ast/expressions.h"
#include "include/ast/translation_unit.h"

class Visitor;

class Statement : public BaseElement {
public:
    virtual void Accept(Visitor* visitor) = 0;
    virtual ~Statement() = default;
};

///////////////////////////////////////////////

class CompoundStatement : public Statement {
public:
    explicit CompoundStatement(std::unique_ptr<ItemList> body);
    virtual ~CompoundStatement() = default;
    void Accept(Visitor* visitor) override;
    ItemList* GetBody();

private:
    std::unique_ptr<ItemList> body_;
};

///////////////////////////////////////////////

class ReturnStatement : public Statement {
public:
    ReturnStatement();
    explicit ReturnStatement(std::unique_ptr<Expression> expression);
    virtual ~ReturnStatement() = default;
    void Accept(Visitor* visitor) override;
    bool HasExpression() const;
    Expression* GetExpression();

private:
    std::optional<std::unique_ptr<Expression>> expression_;
};

///////////////////////////////////////////////

class ExpressionStatement : public Statement {
public:
    ExpressionStatement();
    explicit ExpressionStatement(std::unique_ptr<Expression> expression);
    virtual ~ExpressionStatement() = default;
    void Accept(Visitor* visitor) override;
    bool HasExpression() const;
    Expression* GetExpression() const;

private:
    std::optional<std::unique_ptr<Expression>> expression_;
};
///////////////////////////////////////////////

class SelectionStatement : public Statement {
public:
    explicit SelectionStatement(std::unique_ptr<Expression> cond,
                                std::unique_ptr<Statement> then_stmt);
    explicit SelectionStatement(std::unique_ptr<Expression> cond,
                                std::unique_ptr<Statement> then_stmt,
                                std::unique_ptr<Statement> else_stmt);
    virtual ~SelectionStatement() = default;
    void Accept(Visitor* visitor) override;
    bool HasElseStatement() const;
    Expression* GetCondition() const;
    Statement* GetThenStatement() const;
    Statement* GetElseStatement() const;

private:
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Statement> then_stmt_;
    std::optional<std::unique_ptr<Statement>> else_stmt_;
};

///////////////////////////////////////////////

class JumpStatement : public Statement {
public:
    enum class JumpType {
        kBreak,
        kContinue,
    };

    explicit JumpStatement(JumpType type);
    virtual ~JumpStatement() = default;
    void Accept(Visitor* visitor) override;
    JumpType GetType() const;
    void SetLabel(std::string label);
    std::string GetLabel() const;

private:
    JumpType type_;
    std::string label_;
};

///////////////////////////////////////////////

class WhileStatement : public Statement {
public:
    enum class LoopType {
        kDoWhile,
        kWhile,
    };

    explicit WhileStatement(LoopType type, std::unique_ptr<Expression> cond,
                            std::unique_ptr<Statement> body);
    virtual ~WhileStatement() = default;
    void Accept(Visitor* visitor) override;
    LoopType GetType() const;
    Expression* GetCondition() const;
    Statement* GetBody() const;
    void SetLabel(std::string label);
    std::string GetLabel() const;

private:
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Statement> body_;
    LoopType type_;
    std::string label_;
};
///////////////////////////////////////////////

class ForStatement : public Statement {
public:
    explicit ForStatement(std::unique_ptr<BaseElement> init,
                          std::unique_ptr<Expression> cond,
                          std::unique_ptr<Expression> inc,
                          std::unique_ptr<Statement> body);
    virtual ~ForStatement() = default;
    void Accept(Visitor* visitor) override;
    BaseElement* GetInit() const;
    Expression* GetCondition() const;
    Expression* GetIncrement() const;
    Statement* GetBody() const;
    void SetLabel(std::string label);
    std::string GetLabel() const;

private:
    std::unique_ptr<BaseElement> init_;
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Expression> inc_;
    std::unique_ptr<Statement> body_;
    std::string label_;
};