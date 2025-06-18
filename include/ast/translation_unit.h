#pragma once
#include <vector>

class Visitor;

class BaseElement {
public:
    virtual void Accept(Visitor* visitor) = 0;
};

///////////////////////////////////////////////

class TranslationUnit : public BaseElement {
public:
    TranslationUnit() = default;
    void AddExternalDeclaration(BaseElement* declaration);
    virtual void Accept(Visitor* visitor) override;
    std::vector<BaseElement*>& GetExternalDeclarations();

private:
    std::vector<BaseElement*> external_declarations_;
};

///////////////////////////////////////////////

class ItemList : public BaseElement {
public:
    ItemList() = default;
    void AddItem(BaseElement* item);
    virtual void Accept(Visitor* visitor) override;
    std::vector<BaseElement*>& GetItems();

private:
    std::vector<BaseElement*> items_;
};