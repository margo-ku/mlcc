#pragma once

#include "include/ast/declarations.h"

namespace declarator_utils {

FunctionDeclarator* FindFunctionDeclarator(Declarator* declarator);
IdentifierDeclarator* FindIdentifierDeclarator(Declarator* declarator);
int CountPointerLevelsBeforeFunction(Declarator* declarator);
SingleInitializer* AsSingleInitializer(Initializer* initializer);

}  // namespace declarator_utils
