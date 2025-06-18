#include "include/ast/declarations.h"

#include "include/visitors/visitor.h"

FunctionDefinition::FunctionDefinition(TypeSpecification* return_type,
                                       Declarator* name,
                                       CompoundStatement* body)
    : return_type_(return_type), name_(name), body_(body) {}

void FunctionDefinition::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* FunctionDefinition::GetReturnType() { return return_type_; }
Declarator* FunctionDefinition::GetDeclarator() { return name_; }
CompoundStatement* FunctionDefinition::GetBody() { return body_; }

///////////////////////////////////////////////

Declarator::Declarator(std::string id) : id_(id) {}

std::string Declarator::GetId() const { return id_; }

void Declarator::Accept(Visitor* visitor) { visitor->Visit(this); }

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
