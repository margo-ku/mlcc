#pragma once
#include <optional>

#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor;

///////////////////////////////////////////////

class Declarator : public BaseElement {
public:
    explicit Declarator(std::string id);
    virtual void Accept(Visitor* visitor) override;
    std::string GetId() const;

private:
    std::string id_;
};

///////////////////////////////////////////////

class InitDeclarator : public BaseElement {
public:
    explicit InitDeclarator(Declarator* declarator);
    explicit InitDeclarator(Declarator* declarator, Expression* initializer);
    virtual void Accept(Visitor* visitor) override;
    Declarator* GetDeclarator() const;
    bool HasInitializer() const;
    Expression* GetInitializer() const;

private:
    Declarator* declarator_;
    std::optional<Expression*> initializer_;
};

///////////////////////////////////////////////

class TypeSpecification : public BaseElement {
public:
    enum class Type {
        kInt = 0,
    };
    explicit TypeSpecification(std::string type_name);
    virtual void Accept(Visitor* visitor) override;
    std::string GetTypeName() const;

private:
    Type type_;
};

///////////////////////////////////////////////

class FunctionDefinition : public BaseElement {
public:
    explicit FunctionDefinition(TypeSpecification* return_type, Declarator* name,
                                CompoundStatement* body);
    virtual void Accept(Visitor* visitor) override;
    TypeSpecification* GetReturnType();
    Declarator* GetDeclarator();
    CompoundStatement* GetBody();

private:
    TypeSpecification* return_type_;
    Declarator* name_;
    CompoundStatement* body_;
};

///////////////////////////////////////////////

class Declaration : public BaseElement {
public:
    explicit Declaration(TypeSpecification* type, InitDeclarator* declaration);
    virtual void Accept(Visitor* visitor) override;
    TypeSpecification* GetType() const;
    InitDeclarator* GetDeclaration() const;

private:
    TypeSpecification* type_;
    InitDeclarator* declaration_;
};
