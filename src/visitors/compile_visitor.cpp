#include "include/visitors/compile_visitor.h"

CompileVisitor::CompileVisitor(std::ostream& stream)
    : stream_(stream), number_of_tabs_(0) {}

void CompileVisitor::Visit(TranslationUnit* translation_unit) {
    for (auto* declaration : translation_unit->GetExternalDeclarations()) {
        stream_ << ".globl ";
        dynamic_cast<FunctionDefinition*>(declaration)->GetDeclarator()->Accept(this);
        stream_ << std::endl;
        declaration->Accept(this);
    }
}

void CompileVisitor::Visit(ItemList* item_list) {
    for (auto* item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void CompileVisitor::Visit(FunctionDefinition* function) {
    function->GetDeclarator()->Accept(this);
    stream_ << ":" << std::endl;
    number_of_tabs_++;
    if (function->GetDeclarator()->GetId() == "main") {
        PrintTabs();
        stream_ << "sub sp, sp, #16" << std::endl;
        PrintTabs();
        stream_ << "str wzr, [sp, #12]" << std::endl;
    }

    function->GetBody()->Accept(this);

    if (function->GetDeclarator()->GetId() == "main") {
        PrintTabs();
        stream_ << "add sp, sp, #16" << std::endl;
    }
    PrintTabs();
    stream_ << "ret" << std::endl;
    number_of_tabs_--;
}

void CompileVisitor::Visit(TypeSpecification* type) {}

void CompileVisitor::Visit(Declarator* declarator) {
    stream_ << "_" << declarator->GetId();
}

void CompileVisitor::Visit(PrimaryExpression* expression) {
    stream_ << expression->GetValue() << std::endl;
}

void CompileVisitor::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
    PrintTabs();
    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::kMinus:
            stream_ << "neg w0, w0" << std::endl;
            break;
        case UnaryExpression::UnaryOperator::kBinaryNot:
            stream_ << "mvn w0, w0" << std::endl;
            break;
        case UnaryExpression::UnaryOperator::kNot:
            stream_ << "cmp w0, 0" << std::endl;
            PrintTabs();
            stream_ << "cset w0, eq" << std::endl;
            break;
        case UnaryExpression::UnaryOperator::kPlus:
            break;
    }
}

void CompileVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void CompileVisitor::Visit(ReturnStatement* statement) {
    // to do: has expression
    PrintTabs();
    stream_ << "mov w0, #";
    statement->GetExpression()->Accept(this);
}

void CompileVisitor::PrintTabs() const {
    for (int i = 0; i < number_of_tabs_; ++i) {
        stream_ << '\t';
    }
}