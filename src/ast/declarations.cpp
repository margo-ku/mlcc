#include "include/ast/declarations.h"

#include <stdexcept>

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

std::unique_ptr<Expression> IdentifierDeclarator::ExtractInitializer() {
    if (initializer_.has_value()) {
        return std::move(initializer_.value());
    }
    return nullptr;
}

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
    throw std::invalid_argument("Function declarator cannot have an initializer");
}

Expression* FunctionDeclarator::GetInitializer() const { return nullptr; }

bool FunctionDeclarator::HasInitializer() const { return false; }

///////////////////////////////////////////////

TypeSpecification::TypeSpecification(Type type) : type_(type) {}

TypeSpecification::TypeSpecification(const TypeSpecifierSet& specifiers)
    : type_(ResolveType(specifiers)) {}

TypeSpecification::Type TypeSpecification::GetType() const { return type_; }

TypeSpecification::Type TypeSpecification::ResolveType(const TypeSpecifierSet& set) {
    uint32_t counts_int = set.counts[(int)(TypeSpecifierSet::Specifier::Int)];
    uint32_t counts_long = set.counts[(int)(TypeSpecifierSet::Specifier::Long)];
    uint32_t counts_signed = set.counts[(int)(TypeSpecifierSet::Specifier::Signed)];
    uint32_t counts_unsigned = set.counts[(int)(TypeSpecifierSet::Specifier::Unsigned)];
    uint32_t counts_double = set.counts[(int)(TypeSpecifierSet::Specifier::Double)];

    if (counts_int > 1) {
        throw std::runtime_error(
            "cannot combine 'int' with previous 'int' type specifier");
    } else if (counts_long > 2) {
        throw std::runtime_error(
            "cannot combine 'long' with previous 'long' type specifier");
    } else if (counts_signed > 1) {
        throw std::runtime_error(
            "cannot combine 'signed' with previous 'signed' type specifier");
    } else if (counts_unsigned > 1) {
        throw std::runtime_error(
            "cannot combine 'unsigned' with previous 'unsigned' type specifier");
    } else if (counts_double > 1) {
        throw std::runtime_error(
            "cannot combine 'double' with previous 'double' type specifier");
    }

    if (counts_double == 1 &&
        (counts_int > 0 || counts_long > 0 || counts_signed > 0 || counts_unsigned > 0)) {
        throw std::runtime_error(
            "cannot combine 'double' with previous 'int', 'long', 'signed', or "
            "'unsigned' type specifier");
    }

    if (counts_unsigned > 0 && counts_signed > 0) {
        throw std::runtime_error(
            "cannot combine 'unsigned' with 'signed' type specifier");
    }

    if (counts_unsigned > 0 && counts_long > 0) {
        return Type::ULong;
    } else if (counts_unsigned > 0) {
        return Type::UInt;
    } else if (counts_long > 0) {
        return Type::Long;
    } else if (counts_signed > 0 || counts_int > 0) {
        return Type::Int;
    } else if (counts_double > 0) {
        return Type::Double;
    }
    throw std::runtime_error("invalid type specifier set");
}

std::string TypeSpecification::GetTypeName() const {
    switch (type_) {
        case Type::Int:
            return "int";
        case Type::Long:
            return "long";
        case Type::UInt:
            return "unsigned int";
        case Type::ULong:
            return "unsigned long";
        case Type::Double:
            return "double";
    }
}

void TypeSpecification::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

DeclarationSpecifiers::DeclarationSpecifiers(const DeclarationSpecifierSet& specifiers)
    : type_(std::make_unique<TypeSpecification>(specifiers.type_specifiers)),
      storage_class_(ResolveStorageClass(specifiers.storage_class_specifiers)) {}

void DeclarationSpecifiers::Accept(Visitor* visitor) { visitor->Visit(this); }

TypeSpecification* DeclarationSpecifiers::GetTypeSpecification() const {
    return type_.get();
}

StorageClass DeclarationSpecifiers::GetStorageClass() const { return storage_class_; }

bool DeclarationSpecifiers::HasTypeSpecifier() const { return true; }

bool DeclarationSpecifiers::IsStatic() const {
    return storage_class_ == StorageClass::Static;
}

bool DeclarationSpecifiers::IsExtern() const {
    return storage_class_ == StorageClass::Extern;
}

StorageClass DeclarationSpecifiers::ResolveStorageClass(
    const StorageClassSpecifierSet& specifier) {
    if (specifier.has_static && specifier.has_extern) {
        throw std::runtime_error("cannot combine 'static' and 'extern'");
    }

    if (specifier.has_static) {
        return StorageClass::Static;
    } else if (specifier.has_extern) {
        return StorageClass::Extern;
    }
    return StorageClass::None;
}

///////////////////////////////////////////////

FunctionDefinition::FunctionDefinition(std::unique_ptr<DeclarationSpecifiers> decl_specs,
                                       std::unique_ptr<Declarator> declarator,
                                       std::unique_ptr<CompoundStatement> body)
    : decl_specs_(std::move(decl_specs)),
      declarator_(std::move(declarator)),
      body_(std::move(body)) {}

void FunctionDefinition::Accept(Visitor* visitor) { visitor->Visit(this); }

DeclarationSpecifiers* FunctionDefinition::GetDeclarationSpecifiers() const {
    return decl_specs_.get();
}

TypeSpecification* FunctionDefinition::GetReturnType() const {
    return decl_specs_->GetTypeSpecification();
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

Declaration::Declaration(std::unique_ptr<DeclarationSpecifiers> decl_specs,
                         std::unique_ptr<Declarator> declaration)
    : decl_specs_(std::move(decl_specs)), declaration_(std::move(declaration)) {}

void Declaration::Accept(Visitor* visitor) { visitor->Visit(this); }

DeclarationSpecifiers* Declaration::GetDeclarationSpecifiers() const {
    return decl_specs_.get();
}

TypeSpecification* Declaration::GetType() const {
    return decl_specs_->GetTypeSpecification();
}

Declarator* Declaration::GetDeclaration() const { return declaration_.get(); }

///////////////////////////////////////////////

ParameterDeclaration::ParameterDeclaration(
    std::unique_ptr<DeclarationSpecifiers> decl_specs,
    std::unique_ptr<Declarator> declarator)
    : decl_specs_(std::move(decl_specs)), declarator_(std::move(declarator)) {}

void ParameterDeclaration::Accept(Visitor* visitor) { visitor->Visit(this); }

DeclarationSpecifiers* ParameterDeclaration::GetDeclarationSpecifiers() const {
    return decl_specs_.get();
}

TypeSpecification* ParameterDeclaration::GetType() const {
    return decl_specs_->GetTypeSpecification();
}

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
