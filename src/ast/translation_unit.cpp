#include "include/ast/translation_unit.h"

#include "include/visitors/visitor.h"

void TranslationUnit::AddExternalDeclaration(std::unique_ptr<BaseElement> declaration) {
    if (dynamic_cast<FunctionDefinition*>(declaration.get()) ||
        dynamic_cast<Declaration*>(declaration.get())) {
        external_declarations_.push_back(std::move(declaration));
    } else {
        throw std::invalid_argument(
            "Only FunctionDefinition or Declaration allowed in TranslationUnit");
    }
}

void TranslationUnit::Accept(Visitor* visitor) { visitor->Visit(this); }

const std::vector<std::unique_ptr<BaseElement>>&
TranslationUnit::GetExternalDeclarations() const {
    return external_declarations_;
}

std::vector<std::unique_ptr<BaseElement>>& TranslationUnit::GetExternalDeclarations() {
    return external_declarations_;
}

///////////////////////////////////////////////

void ItemList::AddItem(std::unique_ptr<BaseElement> item) {
    if (dynamic_cast<Statement*>(item.get()) || dynamic_cast<Declaration*>(item.get())) {
        items_.push_back(std::move(item));
    } else {
        throw std::invalid_argument("Only Statement or Declaration allowed in ItemList");
    }
}

void ItemList::Accept(Visitor* visitor) { visitor->Visit(this); }

const std::vector<std::unique_ptr<BaseElement>>& ItemList::GetItems() const {
    return items_;
}

std::vector<std::unique_ptr<BaseElement>>& ItemList::GetItems() { return items_; }
