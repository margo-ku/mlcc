#include "include/semantic/declarator_utils.h"

namespace declarator_utils {

FunctionDeclarator* FindFunctionDeclarator(Declarator* declarator) {
    if (auto* function_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        return function_declarator;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        return FindFunctionDeclarator(pointer_declarator->GetDeclarator());
    }
    return nullptr;
}

IdentifierDeclarator* FindIdentifierDeclarator(Declarator* declarator) {
    if (auto* identifier_declarator =
            dynamic_cast<IdentifierDeclarator*>(declarator)) {
        return identifier_declarator;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        return FindIdentifierDeclarator(pointer_declarator->GetDeclarator());
    }
    if (auto* function_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        return FindIdentifierDeclarator(function_declarator->GetDeclarator());
    }
    return nullptr;
}

int CountPointerLevelsBeforeFunction(Declarator* declarator) {
    if (dynamic_cast<FunctionDeclarator*>(declarator)) {
        return 0;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        return 1 + CountPointerLevelsBeforeFunction(pointer_declarator->GetDeclarator());
    }
    return 0;
}

SingleInitializer* AsSingleInitializer(Initializer* initializer) {
    return dynamic_cast<SingleInitializer*>(initializer);
}

}  // namespace declarator_utils
