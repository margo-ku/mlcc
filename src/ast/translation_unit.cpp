#include "include/ast/translation_unit.h"

#include "include/visitors/visitor.h"

void TranslationUnit::AddExternalDeclaration(BaseElement* declaration) {
    if (dynamic_cast<FunctionDefinition*>(declaration)) {
        external_declarations_.push_back(declaration);
    } else {
        throw std::invalid_argument(
            "Only FunctionDefinition or Declaration allowed in "
            "TranslationUnit");
    }
}

void TranslationUnit::Accept(Visitor* visitor) { visitor->Visit(this); }

std::vector<BaseElement*>& TranslationUnit::GetExternalDeclarations() {
    return external_declarations_;
}

///////////////////////////////////////////////

void ItemList::AddItem(BaseElement* item) {
    if (dynamic_cast<Statement*>(item) || dynamic_cast<Declaration*>(item)) {
        items_.push_back(item);
    } else {
        throw std::invalid_argument("Only Statement or Declaration allowed in ItemList");
    }
}

void ItemList::Accept(Visitor* visitor) { visitor->Visit(this); }

std::vector<BaseElement*>& ItemList::GetItems() { return items_; }

///////////////////////////////////////////////