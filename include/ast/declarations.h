#pragma once
#include <optional>

#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor;

///////////////////////////////////////////////

class Declarator : public BaseElement {
public:
    explicit Declarator();
    virtual ~Declarator() = default;
    virtual void Accept(Visitor* visitor) = 0;
    virtual std::string GetId() const = 0;
    virtual void SetId(const std::string& id) = 0;
    virtual void SetInitializer(std::unique_ptr<Expression> initializer) = 0;
    virtual Expression* GetInitializer() const = 0;
    virtual bool HasInitializer() const = 0;
};

///////////////////////////////////////////////

class IdentifierDeclarator : public Declarator {
public:
    explicit IdentifierDeclarator(std::string id);
    virtual ~IdentifierDeclarator() = default;
    virtual void Accept(Visitor* visitor) override;
    std::string GetId() const override;
    void SetId(const std::string& id) override;
    void SetInitializer(std::unique_ptr<Expression> initializer) override;
    Expression* GetInitializer() const override;
    bool HasInitializer() const override;
    std::unique_ptr<Expression> ExtractInitializer();

private:
    std::string id_;
    std::optional<std::unique_ptr<Expression>> initializer_;
};

///////////////////////////////////////////////

class ParameterList;

class FunctionDeclarator : public Declarator {
public:
    explicit FunctionDeclarator(std::unique_ptr<Declarator> declarator);
    explicit FunctionDeclarator(std::unique_ptr<Declarator> declarator,
                                std::unique_ptr<ParameterList> parameters);
    virtual ~FunctionDeclarator() = default;
    void Accept(Visitor* visitor) override;
    Declarator* GetDeclarator() const;
    ParameterList* GetParameters() const;
    bool HasParameters() const;
    std::string GetId() const override;
    void SetId(const std::string& id) override;
    void SetInitializer(std::unique_ptr<Expression> initializer) override;
    Expression* GetInitializer() const override;
    bool HasInitializer() const override;

private:
    std::unique_ptr<Declarator> declarator_;
    std::optional<std::unique_ptr<ParameterList>> parameters_;
};

///////////////////////////////////////////////

class TypeSpecification : public BaseElement {
public:
    enum class Type {
        Int = 0,
        Long = 1,
    };
    explicit TypeSpecification(Type type);
    virtual ~TypeSpecification() = default;
    void Accept(Visitor* visitor) override;
    std::string GetTypeName() const;
    Type GetType() const;

private:
    Type type_;
};

///////////////////////////////////////////////

class FunctionDefinition : public BaseElement {
public:
    FunctionDefinition(std::unique_ptr<TypeSpecification> return_type,
                       std::unique_ptr<Declarator> declarator,
                       std::unique_ptr<CompoundStatement> body);
    virtual ~FunctionDefinition() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetReturnType() const;
    Declarator* GetDeclarator() const;
    CompoundStatement* GetBody() const;
    int GetNumParameters() const;

private:
    std::unique_ptr<TypeSpecification> return_type_;
    std::unique_ptr<Declarator> declarator_;
    std::unique_ptr<CompoundStatement> body_;
};

///////////////////////////////////////////////

class Declaration : public BaseElement {
public:
    Declaration(std::unique_ptr<TypeSpecification> type,
                std::unique_ptr<Declarator> declaration);
    virtual ~Declaration() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetType() const;
    Declarator* GetDeclaration() const;

private:
    std::unique_ptr<TypeSpecification> type_;
    std::unique_ptr<Declarator> declaration_;
};

///////////////////////////////////////////////

class ParameterDeclaration : public BaseElement {
public:
    ParameterDeclaration(std::unique_ptr<TypeSpecification> type,
                         std::unique_ptr<Declarator> declarator);
    virtual ~ParameterDeclaration() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetType() const;
    Declarator* GetDeclarator() const;

private:
    std::unique_ptr<TypeSpecification> type_;
    std::unique_ptr<Declarator> declarator_;
};

///////////////////////////////////////////////

class ParameterList : public BaseElement {
public:
    ParameterList();
    virtual ~ParameterList() = default;
    void Accept(Visitor* visitor) override;
    void AddParameter(std::unique_ptr<ParameterDeclaration> parameter);
    const std::vector<std::unique_ptr<ParameterDeclaration>>& GetParameters() const;
    std::vector<std::unique_ptr<ParameterDeclaration>>& GetParameters();

private:
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters_;
};
