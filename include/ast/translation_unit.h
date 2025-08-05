#pragma once
#include <vector>

class Visitor;

class BaseElement {
public:
    virtual void Accept(Visitor* visitor) = 0;
    virtual ~BaseElement() = default;
};

///////////////////////////////////////////////

class TranslationUnit : public BaseElement {
public:
    TranslationUnit() = default;
    virtual ~TranslationUnit() = default;
    void AddExternalDeclaration(std::unique_ptr<BaseElement> declaration);
    virtual void Accept(Visitor* visitor) override;
    const std::vector<std::unique_ptr<BaseElement>>& GetExternalDeclarations() const;
    std::vector<std::unique_ptr<BaseElement>>& GetExternalDeclarations();

private:
    std::vector<std::unique_ptr<BaseElement>> external_declarations_;
};

///////////////////////////////////////////////

class ItemList : public BaseElement {
public:
    ItemList() = default;
    virtual ~ItemList() = default;

    void AddItem(std::unique_ptr<BaseElement> item);
    void Accept(Visitor* visitor) override;

    const std::vector<std::unique_ptr<BaseElement>>& GetItems() const;
    std::vector<std::unique_ptr<BaseElement>>& GetItems();

private:
    std::vector<std::unique_ptr<BaseElement>> items_;
};