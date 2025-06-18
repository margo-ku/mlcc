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
    PrintToStream("mov w0, #" + std::to_string(expression->GetValue()));
}

void CompileVisitor::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::kMinus:
            PrintToStream("neg w0, w0");
            break;
        case UnaryExpression::UnaryOperator::kBinaryNot:
            PrintToStream("mvn w0, w0");
            break;
        case UnaryExpression::UnaryOperator::kNot:
            PrintToStream("cmp w0, 0");
            PrintToStream("cset w0, eq");
            break;
        case UnaryExpression::UnaryOperator::kPlus:
            break;
    }
}

void CompileVisitor::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);  // result in w0
    PrintToStream("str w0, [sp, #-16]!");
    expression->GetRightExpression()->Accept(this);  // result in w0
    PrintToStream("ldr w1, [sp], #16");

    // left in w1, right in w0
    switch (expression->GetOp()) {
        case BinaryExpression::BinaryOperator::kPlus:
            PrintToStream("add w0, w0, w1");
            break;
        case BinaryExpression::BinaryOperator::kMinus:
            PrintToStream("sub w0, w1, w0");
            break;
        case BinaryExpression::BinaryOperator::kMul:
            PrintToStream("mul w0, w1, w0");
            break;
        case BinaryExpression::BinaryOperator::kDiv:
            PrintToStream("sdiv w0, w1, w0");
            break;
        case BinaryExpression::BinaryOperator::kMod:
            PrintToStream("sdiv w2, w1, w0");
            PrintToStream("msub w0, w2, w0, w1");
            break;
        default:
            break;
    }
}

void CompileVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void CompileVisitor::Visit(ReturnStatement* statement) {
    // to do: has expression
    statement->GetExpression()->Accept(this);
}

void CompileVisitor::PrintTabs() const {
    for (int i = 0; i < number_of_tabs_; ++i) {
        stream_ << '\t';
    }
}

void CompileVisitor::PrintToStream(const std::string& output) const {
    PrintTabs();
    stream_ << output << std::endl;
}