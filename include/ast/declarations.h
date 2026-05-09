#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "include/ast/expressions.h"
#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor;

///////////////////////////////////////////////

class Initializer;

class Declarator : public BaseElement {
public:
    explicit Declarator();
    virtual ~Declarator() = default;
    virtual void Accept(Visitor* visitor) = 0;

    virtual std::string GetId() const = 0;
    virtual void SetId(const std::string& id) = 0;
    virtual void SetInitializer(std::unique_ptr<Initializer> initializer) = 0;
    virtual Initializer* GetInitializer() const = 0;
    virtual bool HasInitializer() const = 0;
};

///////////////////////////////////////////////

class IdentifierDeclarator : public Declarator {
public:
    explicit IdentifierDeclarator(std::string id);
    virtual ~IdentifierDeclarator() = default;
    void Accept(Visitor* visitor) override;

    std::string GetId() const override;
    void SetId(const std::string& id) override;
    void SetInitializer(std::unique_ptr<Initializer> initializer) override;
    Initializer* GetInitializer() const override;
    bool HasInitializer() const override;

    std::unique_ptr<Initializer> ExtractInitializer();

private:
    std::string id_;
    std::optional<std::unique_ptr<Initializer>> initializer_;
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

    std::string GetId() const override;
    void SetId(const std::string& id) override;
    void SetInitializer(std::unique_ptr<Initializer> initializer) override;
    Initializer* GetInitializer() const override;
    bool HasInitializer() const override;

    Declarator* GetDeclarator() const;
    ParameterList* GetParameters() const;
    bool HasParameters() const;

private:
    std::unique_ptr<Declarator> declarator_;
    std::optional<std::unique_ptr<ParameterList>> parameters_;
};

///////////////////////////////////////////////

class PointerDeclarator : public Declarator {
public:
    explicit PointerDeclarator(std::unique_ptr<Declarator> declarator);
    virtual ~PointerDeclarator() = default;
    void Accept(Visitor* visitor) override;

    std::string GetId() const override;
    void SetId(const std::string& id) override;
    void SetInitializer(std::unique_ptr<Initializer> initializer) override;
    Initializer* GetInitializer() const override;
    bool HasInitializer() const override;

    Declarator* GetDeclarator() const;

private:
    std::unique_ptr<Declarator> declarator_;
};

///////////////////////////////////////////////

class ArrayDeclarator : public PointerDeclarator {
public:
    explicit ArrayDeclarator(std::unique_ptr<Declarator> declarator,
                             std::unique_ptr<Expression> size);
    virtual ~ArrayDeclarator() = default;
    void Accept(Visitor* visitor) override;

    Expression* GetSize() const;

private:
    std::unique_ptr<Expression> size_;
};

///////////////////////////////////////////////

struct TypeSpecifierSet {
    enum class Specifier {
        Int = 0,
        Long = 1,
        Signed = 2,
        Unsigned = 3,
        Double = 4,
    };
    std::vector<uint32_t> counts;

    // to do: update counts len
    TypeSpecifierSet() : counts(5) {}

    void Add(Specifier spec) { counts[(int)spec]++; }
};

struct StorageClassSpecifierSet {
    enum class Specifier { Static, Extern };

    bool has_static = false;
    bool has_extern = false;

    void Add(Specifier spec) {
        switch (spec) {
            case Specifier::Static:
                has_static = true;
                break;
            case Specifier::Extern:
                has_extern = true;
                break;
        }
    }
};

struct DeclarationSpecifierSet {
    TypeSpecifierSet type_specifiers;
    StorageClassSpecifierSet storage_class_specifiers;

    void Add(TypeSpecifierSet::Specifier spec) { type_specifiers.Add(spec); }

    void Add(StorageClassSpecifierSet::Specifier spec) {
        storage_class_specifiers.Add(spec);
    }
};

///////////////////////////////////////////////

class TypeSpecification : public BaseElement {
public:
    enum class Type {
        Int = 0,
        Long = 1,
        UInt = 2,
        ULong = 3,
        Double = 4,
    };

    explicit TypeSpecification(Type type);
    explicit TypeSpecification(const TypeSpecifierSet& specifiers);
    virtual ~TypeSpecification() = default;
    void Accept(Visitor* visitor) override;
    std::string GetTypeName() const;
    Type GetType() const;

    static Type ResolveType(const TypeSpecifierSet& specifiers);

private:
    Type type_;
};

///////////////////////////////////////////////

enum class StorageClass { None, Static, Extern };

class DeclarationSpecifiers : public BaseElement {
public:
    explicit DeclarationSpecifiers(const DeclarationSpecifierSet& specifiers);
    virtual ~DeclarationSpecifiers() = default;
    void Accept(Visitor* visitor) override;

    TypeSpecification* GetTypeSpecification() const;
    StorageClass GetStorageClass() const;
    bool HasTypeSpecifier() const;
    bool IsStatic() const;
    bool IsExtern() const;

    static StorageClass ResolveStorageClass(const StorageClassSpecifierSet& specifiers);

private:
    std::unique_ptr<TypeSpecification> type_;
    StorageClass storage_class_ = StorageClass::None;
};

///////////////////////////////////////////////

class FunctionDefinition : public BaseElement {
public:
    FunctionDefinition(std::unique_ptr<DeclarationSpecifiers> decl_specs,
                       std::unique_ptr<Declarator> declarator,
                       std::unique_ptr<CompoundStatement> body);
    virtual ~FunctionDefinition() = default;
    void Accept(Visitor* visitor) override;
    DeclarationSpecifiers* GetDeclarationSpecifiers() const;
    TypeSpecification* GetReturnType() const;
    Declarator* GetDeclarator() const;
    CompoundStatement* GetBody() const;
    int GetNumParameters() const;

private:
    std::unique_ptr<DeclarationSpecifiers> decl_specs_;
    std::unique_ptr<Declarator> declarator_;
    std::unique_ptr<CompoundStatement> body_;
};

///////////////////////////////////////////////

class Declaration : public BaseElement {
public:
    Declaration(std::unique_ptr<DeclarationSpecifiers> decl_specs,
                std::unique_ptr<Declarator> declaration);
    virtual ~Declaration() = default;
    void Accept(Visitor* visitor) override;
    DeclarationSpecifiers* GetDeclarationSpecifiers() const;
    TypeSpecification* GetType() const;
    Declarator* GetDeclaration() const;

private:
    std::unique_ptr<DeclarationSpecifiers> decl_specs_;
    std::unique_ptr<Declarator> declaration_;
};

///////////////////////////////////////////////

class ParameterDeclaration : public BaseElement {
public:
    ParameterDeclaration(std::unique_ptr<DeclarationSpecifiers> decl_specs,
                         std::unique_ptr<Declarator> declarator);
    virtual ~ParameterDeclaration() = default;
    void Accept(Visitor* visitor) override;
    DeclarationSpecifiers* GetDeclarationSpecifiers() const;
    TypeSpecification* GetType() const;
    Declarator* GetDeclarator() const;

private:
    std::unique_ptr<DeclarationSpecifiers> decl_specs_;
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

///////////////////////////////////////////////

class AbstractDeclarator : public BaseElement {
public:
    virtual ~AbstractDeclarator() = default;
    virtual void Accept(Visitor* visitor) = 0;
};

///////////////////////////////////////////////

class PointerAbstractDeclarator : public AbstractDeclarator {
public:
    explicit PointerAbstractDeclarator(
        std::unique_ptr<AbstractDeclarator> base = nullptr);
    virtual ~PointerAbstractDeclarator() = default;
    void Accept(Visitor* visitor) override;

    AbstractDeclarator* GetBase() const;
    bool HasBase() const;

private:
    std::unique_ptr<AbstractDeclarator> base_;
};

///////////////////////////////////////////////

class ArrayAbstractDeclarator : public AbstractDeclarator {
public:
    explicit ArrayAbstractDeclarator(std::unique_ptr<Expression> size,
                                     std::unique_ptr<AbstractDeclarator> base = nullptr);
    virtual ~ArrayAbstractDeclarator() = default;
    void Accept(Visitor* visitor) override;

    Expression* GetSize() const;
    AbstractDeclarator* GetBase() const;
    bool HasBase() const;

private:
    std::unique_ptr<Expression> size_;
    std::unique_ptr<AbstractDeclarator> base_;
};

///////////////////////////////////////////////

class TypeName : public BaseElement {
public:
    explicit TypeName(const TypeSpecifierSet& specifiers);
    TypeName(const TypeSpecifierSet& specifiers,
             std::unique_ptr<AbstractDeclarator> abstract_declarator);
    virtual ~TypeName() = default;
    void Accept(Visitor* visitor) override;

    TypeSpecification* GetTypeSpecification() const;
    AbstractDeclarator* GetAbstractDeclarator() const;
    bool HasAbstractDeclarator() const;

private:
    std::unique_ptr<TypeSpecification> type_spec_;
    std::unique_ptr<AbstractDeclarator> abstract_declarator_;
};

///////////////////////////////////////////////

class Initializer : public BaseElement {
public:
    virtual ~Initializer() = default;
    virtual void Accept(Visitor* visitor);
    TypeRef GetTypeRef() const;
    void SetTypeRef(TypeRef type);

private:
    TypeRef type_ref_;
};

class CompoundInitializer : public Initializer {
public:
    explicit CompoundInitializer();
    virtual ~CompoundInitializer() = default;
    void Accept(Visitor* visitor) override;

    void AddInitializer(std::unique_ptr<Initializer> initializer);
    const std::vector<std::unique_ptr<Initializer>>& GetInitializers() const;
    std::vector<std::unique_ptr<Initializer>>& GetInitializers();

private:
    std::vector<std::unique_ptr<Initializer>> initializers_;
};

class SingleInitializer : public Initializer {
public:
    explicit SingleInitializer(std::unique_ptr<Expression> expression);
    virtual ~SingleInitializer() = default;
    void Accept(Visitor* visitor) override;

    Expression* GetExpression() const;
    void SetExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractExpression();

private:
    std::unique_ptr<Expression> expression_;
};

///////////////////////////////////////////////