#include "include/ast/declarations.h"

#include "include/visitors/visitor.h"

Declarator::Declarator(std::string id) : id_(id) {}

std::string Declarator::GetId() const { return id_; }

void Declarator::SetId(const std::string& id) { id_ = id; }

void Declarator::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

InitDeclarator::InitDeclarator(std::unique_ptr<Declarator> declarator)
    : declarator_(std::move(declarator)) {}

InitDeclarator::InitDeclarator(std::unique_ptr<Declarator> declarator,
                               std::unique_ptr<Expression> initializer)
    : declarator_(std::move(declarator)), initializer_(std::move(initializer)) {}

void InitDeclarator::Accept(Visitor* visitor) { visitor->Visit(this); }

Declarator* InitDeclarator::GetDeclarator() const { return declarator_.get(); }

bool InitDeclarator::HasInitializer() const { return initializer_.has_value(); }

Expression* InitDeclarator::GetInitializer() const {
    return initializer_ ? initializer_->get() : nullptr;
}

///////////////////////////////////////////////

TypeSpecification::TypeSpecification(std::string type_name) {
    if (type_name == "int") {
        type_ = Type::Int;
    } else {
        throw std::invalid_argument("Unknown type specifier: " + type_name);
    }
}

std::string TypeSpecification::GetTypeName() const { return "int"; }

void TypeSpecification::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

FunctionDefinition::FunctionDefinition(std::unique_ptr<TypeSpecification> return_type,
                                       std::unique_ptr<Declarator> name,
                                       std::unique_ptr<CompoundStatement> body)
    : return_type_(std::move(return_type)),
      name_(std::move(name)),
      body_(std::move(body)) {}

void FunctionDefinition::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* FunctionDefinition::GetReturnType() const { return return_type_.get(); }

Declarator* FunctionDefinition::GetDeclarator() const { return name_.get(); }

CompoundStatement* FunctionDefinition::GetBody() const { return body_.get(); }

///////////////////////////////////////////////

Declaration::Declaration(std::unique_ptr<TypeSpecification> type,
                         std::unique_ptr<InitDeclarator> declaration)
    : type_(std::move(type)), declaration_(std::move(declaration)) {}

void Declaration::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* Declaration::GetType() const { return type_.get(); }

InitDeclarator* Declaration::GetDeclaration() const { return declaration_.get(); }