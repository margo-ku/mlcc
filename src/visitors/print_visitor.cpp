#include "include/visitors/print_visitor.h"

PrintVisitor::PrintVisitor(std::ostream& stream) : stream_(stream), number_of_tabs_(0) {}

void PrintVisitor::Visit(TranslationUnit* translation_unit) {
    stream_ << "TranslationUnit:" << std::endl;
    number_of_tabs_++;

    for (auto* declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }

    number_of_tabs_--;
}

void PrintVisitor::Visit(ItemList* item_list) {
    for (auto* item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void PrintVisitor::Visit(FunctionDefinition* function) {
    PrintTabs();
    stream_ << "FunctionDefinition: function ";
    function->GetDeclarator()->Accept(this);
    stream_ << " with return type ";
    function->GetReturnType()->Accept(this);
    stream_ << std::endl;

    number_of_tabs_++;
    function->GetBody()->Accept(this);
    number_of_tabs_--;
}

void PrintVisitor::Visit(TypeSpecification* type) { stream_ << type->GetTypeName(); }

void PrintVisitor::Visit(Declarator* declarator) { stream_ << declarator->GetId(); }

void PrintVisitor::Visit(PrimaryExpression* expression) {
    stream_ << expression->GetValue();
}

void PrintVisitor::Visit(UnaryExpression* expression) {
    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::kMinus:
            stream_ << "-";
            break;
        case UnaryExpression::UnaryOperator::kBinaryNot:
            stream_ << "~";
            break;
        case UnaryExpression::UnaryOperator::kNot:
            stream_ << "!";
            break;
        case UnaryExpression::UnaryOperator::kPlus:
            stream_ << "+";
            break;
    }
}

void PrintVisitor::Visit(CompoundStatement* statement) {
    PrintTabs();
    stream_ << "CompoundStatement:" << std::endl;
    number_of_tabs_++;
    statement->GetBody()->Accept(this);
    number_of_tabs_--;
}

void PrintVisitor::Visit(ReturnStatement* statement) {
    PrintTabs();
    stream_ << "ReturnStatement: return";
    if (statement->HasExpression()) {
        stream_ << " ";
        statement->GetExpression()->Accept(this);
    }
    stream_ << std::endl;
}

void PrintVisitor::PrintTabs() const {
    for (int i = 0; i < number_of_tabs_; ++i) {
        stream_ << '\t';
    }
}