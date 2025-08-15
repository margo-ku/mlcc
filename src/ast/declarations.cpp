#include "include/ast/declarations.h"

#include "include/visitors/visitor.h"

Declarator::Declarator() {}

///////////////////////////////////////////////

IdentifierDeclarator::IdentifierDeclarator(std::string id) : id_(id) {}

void IdentifierDeclarator::Accept(Visitor* visitor) { visitor->Visit(this); }

std::string IdentifierDeclarator::GetId() const { return id_; }

void IdentifierDeclarator::SetId(const std::string& id) { id_ = id; }

void IdentifierDeclarator::SetInitializer(std::unique_ptr<Expression> initializer) {
    initializer_ = std::move(initializer);
}

Expression* IdentifierDeclarator::GetInitializer() const { return initializer_->get(); }

bool IdentifierDeclarator::HasInitializer() const { return initializer_.has_value(); }

///////////////////////////////////////////////

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator)
    : declarator_(std::move(declarator)) {}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<Declarator> declarator,
                                       std::unique_ptr<ParameterList> parameters)
    : declarator_(std::move(declarator)), parameters_(std::move(parameters)) {}

void FunctionDeclarator::Accept(Visitor* visitor) { visitor->Visit(this); }

Declarator* FunctionDeclarator::GetDeclarator() const { return declarator_.get(); }

ParameterList* FunctionDeclarator::GetParameters() const {
    return parameters_.has_value() ? parameters_.value().get() : nullptr;
}

bool FunctionDeclarator::HasParameters() const { return parameters_.has_value(); }

std::string FunctionDeclarator::GetId() const { return declarator_->GetId(); }

void FunctionDeclarator::SetId(const std::string& id) { declarator_->SetId(id); }

void FunctionDeclarator::SetInitializer(std::unique_ptr<Expression> initializer) {
    // todo: exception?
    throw std::invalid_argument("Function declarator cannot have an initializer");
}

Expression* FunctionDeclarator::GetInitializer() const { return nullptr; }

bool FunctionDeclarator::HasInitializer() const { return false; }

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
                                       std::unique_ptr<Declarator> declarator,
                                       std::unique_ptr<CompoundStatement> body)
    : return_type_(std::move(return_type)),
      declarator_(std::move(declarator)),
      body_(std::move(body)) {}

void FunctionDefinition::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* FunctionDefinition::GetReturnType() const {
    return return_type_.get();
}

Declarator* FunctionDefinition::GetDeclarator() const { return declarator_.get(); }

CompoundStatement* FunctionDefinition::GetBody() const { return body_.get(); }

int FunctionDefinition::GetNumParameters() const {
    return dynamic_cast<FunctionDeclarator*>(declarator_.get())
        ->GetParameters()
        ->GetParameters()
        .size();
}

///////////////////////////////////////////////

Declaration::Declaration(std::unique_ptr<TypeSpecification> type,
                         std::unique_ptr<Declarator> declaration)
    : type_(std::move(type)), declaration_(std::move(declaration)) {}

void Declaration::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* Declaration::GetType() const { return type_.get(); }

Declarator* Declaration::GetDeclaration() const { return declaration_.get(); }

///////////////////////////////////////////////

ParameterDeclaration::ParameterDeclaration(std::unique_ptr<TypeSpecification> type,
                                           std::unique_ptr<Declarator> declarator)
    : type_(std::move(type)), declarator_(std::move(declarator)) {}

void ParameterDeclaration::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* ParameterDeclaration::GetType() const { return type_.get(); }

Declarator* ParameterDeclaration::GetDeclarator() const { return declarator_.get(); }

///////////////////////////////////////////////

ParameterList::ParameterList() {}

void ParameterList::Accept(Visitor* visitor) { visitor->Visit(this); }

const std::vector<std::unique_ptr<ParameterDeclaration>>& ParameterList::GetParameters()
    const {
    return parameters_;
}

std::vector<std::unique_ptr<ParameterDeclaration>>& ParameterList::GetParameters() {
    return parameters_;
}

void ParameterList::AddParameter(std::unique_ptr<ParameterDeclaration> parameter) {
    parameters_.push_back(std::move(parameter));
}
