#include "include/ast/declarations.h"

#include "include/visitors/visitor.h"

FunctionDefinition::FunctionDefinition(TypeSpecification* return_type, Declarator* name,
                                       CompoundStatement* body)
    : return_type_(return_type), name_(name), body_(body) {}

void FunctionDefinition::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* FunctionDefinition::GetReturnType() { return return_type_; }
Declarator* FunctionDefinition::GetDeclarator() { return name_; }
CompoundStatement* FunctionDefinition::GetBody() { return body_; }

///////////////////////////////////////////////

Declarator::Declarator(std::string id) : id_(id) {}

std::string Declarator::GetId() const { return id_; }

void Declarator::SetId(const std::string& id) { id_ = id; }

void Declarator::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

InitDeclarator::InitDeclarator(Declarator* declarator) : declarator_(declarator) {}

InitDeclarator::InitDeclarator(Declarator* declarator, Expression* initializer)
    : declarator_(declarator), initializer_(initializer) {}

void InitDeclarator::Accept(Visitor* visitor) { visitor->Visit(this); }

Declarator* InitDeclarator::GetDeclarator() const { return declarator_; }

bool InitDeclarator::HasInitializer() const { return initializer_.has_value(); }

Expression* InitDeclarator::GetInitializer() const { return initializer_.value(); }

///////////////////////////////////////////////

TypeSpecification::TypeSpecification(std::string type_name) {
    if (type_name == "int") {
        type_ = Type::kInt;
    } else {
        throw std::invalid_argument("Unknown type specifier: " + type_name);
    }
}

std::string TypeSpecification::GetTypeName() const { return "int"; }

void TypeSpecification::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

Declaration::Declaration(TypeSpecification* type, InitDeclarator* declaration)
    : type_(type), declaration_(declaration) {}

void Declaration::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* Declaration::GetType() const { return type_; }

InitDeclarator* Declaration::GetDeclaration() const { return declaration_; };