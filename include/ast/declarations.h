#pragma once
#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor;

// class Declaration

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
    explicit FunctionDefinition(TypeSpecification* return_type,
                                Declarator* name, CompoundStatement* body);
    virtual void Accept(Visitor* visitor) override;
    TypeSpecification* GetReturnType();
    Declarator* GetDeclarator();
    CompoundStatement* GetBody();

private:
    TypeSpecification* return_type_;
    Declarator* name_;
    CompoundStatement* body_;
};