#include "include/visitors/compile_visitor.h"

CompileVisitor::CompileVisitor(std::ostream& stream, FrameInfo frame_info)
    : stream_(stream),
      frame_info_(frame_info),
      number_of_tabs_(0),
      unique_label_id_(0),
      function_exit_label_("exit_0") {}

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

    PrintToStream("stp x29, x30, [sp, #-16]!");
    PrintToStream("mov x29, sp");
    PrintToStream("sub sp, sp, " + std::to_string(frame_info_.total_size));

    function->GetBody()->Accept(this);

    PrintToStream("mov w0, #0");
    PrintToStream(function_exit_label_ + ": ");
    PrintToStream("add sp, sp, " + std::to_string(frame_info_.total_size));
    PrintToStream("mov sp, x29");
    PrintToStream("ldp x29, x30, [sp], #16");
    PrintToStream("ret");
    number_of_tabs_--;
}

void CompileVisitor::Visit(TypeSpecification* type) {}

void CompileVisitor::Visit(Declarator* declarator) {
    /* to do */
    stream_ << "_" << declarator->GetId();
}

void CompileVisitor::Visit(InitDeclarator* declarator) {
    std::string id = declarator->GetDeclarator()->GetId();
    int offset = frame_info_.variables[0].at(id).offset;

    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
    PrintToStream("str w0, [x29, #" + std::to_string(offset) + "]");
}

void CompileVisitor::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void CompileVisitor::Visit(IdExpression* expression) {
    int offset = frame_info_.variables[0].at(expression->GetId()).offset;
    PrintToStream("ldr w0, [x29, #" + std::to_string(offset) + "]");
}

void CompileVisitor::Visit(PrimaryExpression* expression) {
    // только для маленьких чисел (пока): to do
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
    if (expression->GetOp() == BinaryExpression::BinaryOperator::kOr) {
        ProcessBinaryOr(expression);
        return;
    }

    if (expression->GetOp() == BinaryExpression::BinaryOperator::kAnd) {
        ProcessBinaryAnd(expression);
        return;
    }

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
        case BinaryExpression::BinaryOperator::kLess:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, lt");
            break;
        case BinaryExpression::BinaryOperator::kGreater:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, gt");
            break;
        case BinaryExpression::BinaryOperator::kLessEqual:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, le");
            break;
        case BinaryExpression::BinaryOperator::kGreaterEqual:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, ge");
            break;
        case BinaryExpression::BinaryOperator::kEqual:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, eq");
            break;
        case BinaryExpression::BinaryOperator::kNotEqual:
            PrintToStream("cmp w1, w0");
            PrintToStream("cset w0, ne");
            break;
        default:
            break;
    }
}

void CompileVisitor::Visit(AssignmentExpression* expression) {
    std::string id =
        dynamic_cast<IdExpression*>(expression->GetLeftExpression())->GetId();
    int offset = frame_info_.variables[0].at(id).offset;

    expression->GetRightExpression()->Accept(this);
    PrintToStream("str w0, [x29, #" + std::to_string(offset) + "]");
}

void CompileVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void CompileVisitor::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
    PrintToStream("b " + function_exit_label_);
}

void CompileVisitor::Visit(ExpressionStatement* statement) {
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

void CompileVisitor::ProcessBinaryOr(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_set_true = "label_set_true_" + label_id;
    std::string label_end = "label_end_" + label_id;

    expression->GetLeftExpression()->Accept(this);
    PrintToStream("cmp w0, #0");
    PrintToStream("bne " + label_set_true);
    expression->GetRightExpression()->Accept(this);
    PrintToStream("cmp w0, #0");
    PrintToStream("bne " + label_set_true);

    PrintToStream("mov w0, #0");
    PrintToStream("b " + label_end);

    PrintToStream(label_set_true + ":");
    PrintToStream("mov w0, #1");

    PrintToStream(label_end + ":");
}

void CompileVisitor::ProcessBinaryAnd(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_set_false = "label_set_false_" + label_id;
    std::string label_end = "label_end_" + label_id;

    expression->GetLeftExpression()->Accept(this);
    PrintToStream("cmp w0, #0");
    PrintToStream("beq " + label_set_false);
    expression->GetRightExpression()->Accept(this);
    PrintToStream("cmp w0, #0");
    PrintToStream("beq " + label_set_false);

    PrintToStream("mov w0, #1");
    PrintToStream("b " + label_end);

    PrintToStream(label_set_false + ":");
    PrintToStream("mov w0, #0");

    PrintToStream(label_end + ":");
}

std::string CompileVisitor::GetUniqueLabelId() {
    return std::to_string(unique_label_id_++);
}