#pragma once
#include <optional>

#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor;

///////////////////////////////////////////////

class Declarator : public BaseElement {
public:
    explicit Declarator(std::string id);
    virtual ~Declarator() = default;
    virtual void Accept(Visitor* visitor) override;
    std::string GetId() const;
    void SetId(const std::string& id);

private:
    std::string id_;
};

///////////////////////////////////////////////

class InitDeclarator : public BaseElement {
public:
    explicit InitDeclarator(std::unique_ptr<Declarator> declarator);
    InitDeclarator(std::unique_ptr<Declarator> declarator,
                   std::unique_ptr<Expression> initializer);
    virtual ~InitDeclarator() = default;

    void Accept(Visitor* visitor) override;
    Declarator* GetDeclarator() const;
    bool HasInitializer() const;
    Expression* GetInitializer() const;

private:
    std::unique_ptr<Declarator> declarator_;
    std::optional<std::unique_ptr<Expression>> initializer_;
};

///////////////////////////////////////////////

class TypeSpecification : public BaseElement {
public:
    enum class Type {
        kInt = 0,
    };
    explicit TypeSpecification(std::string type_name);
    virtual ~TypeSpecification() = default;
    void Accept(Visitor* visitor) override;
    std::string GetTypeName() const;

private:
    Type type_;
};

///////////////////////////////////////////////

class FunctionDefinition : public BaseElement {
public:
    FunctionDefinition(std::unique_ptr<TypeSpecification> return_type,
                       std::unique_ptr<Declarator> name,
                       std::unique_ptr<CompoundStatement> body);
    virtual ~FunctionDefinition() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetReturnType();
    Declarator* GetDeclarator();
    CompoundStatement* GetBody();

private:
    std::unique_ptr<TypeSpecification> return_type_;
    std::unique_ptr<Declarator> name_;
    std::unique_ptr<CompoundStatement> body_;
};

///////////////////////////////////////////////

class Declaration : public BaseElement {
public:
    Declaration(std::unique_ptr<TypeSpecification> type,
                std::unique_ptr<InitDeclarator> declaration);
    virtual ~Declaration() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetType() const;
    InitDeclarator* GetDeclaration() const;

private:
    std::unique_ptr<TypeSpecification> type_;
    std::unique_ptr<InitDeclarator> declaration_;
};