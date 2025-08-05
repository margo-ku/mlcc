#include "include/visitors/print_visitor.h"

PrintVisitor::PrintVisitor(std::ostream& stream) : stream_(stream), number_of_tabs_(0) {}

void PrintVisitor::Visit(TranslationUnit* translation_unit) {
    stream_ << "TranslationUnit:" << std::endl;
    number_of_tabs_++;

    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }

    number_of_tabs_--;
}

void PrintVisitor::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
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

void PrintVisitor::Visit(InitDeclarator* declarator) {
    declarator->GetDeclarator()->Accept(this);
    if (declarator->HasInitializer()) {
        stream_ << " = ";
        declarator->GetInitializer()->Accept(this);
    }
}

void PrintVisitor::Visit(Declaration* declaration) {
    PrintTabs();
    stream_ << "Declaraion: ";
    declaration->GetType()->Accept(this);
    stream_ << " ";
    declaration->GetDeclaration()->Accept(this);
    stream_ << std::endl;
}

void PrintVisitor::Visit(Expression* expression) {}

void PrintVisitor::Visit(IdExpression* expression) { stream_ << expression->GetId(); }

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
    expression->GetExpression()->Accept(this);
}

void PrintVisitor::Visit(BinaryExpression* expression) {
    stream_ << "(";
    expression->GetLeftExpression()->Accept(this);
    switch (expression->GetOp()) {
        case BinaryExpression::BinaryOperator::kPlus:
            stream_ << " + ";
            break;
        case BinaryExpression::BinaryOperator::kMinus:
            stream_ << " - ";
            break;
        case BinaryExpression::BinaryOperator::kMul:
            stream_ << " * ";
            break;
        case BinaryExpression::BinaryOperator::kDiv:
            stream_ << " / ";
            break;
        case BinaryExpression::BinaryOperator::kMod:
            stream_ << " % ";
            break;
        case BinaryExpression::BinaryOperator::kLess:
            stream_ << " < ";
            break;
        case BinaryExpression::BinaryOperator::kGreater:
            stream_ << " > ";
            break;
        case BinaryExpression::BinaryOperator::kLessEqual:
            stream_ << " <= ";
            break;
        case BinaryExpression::BinaryOperator::kGreaterEqual:
            stream_ << " >= ";
            break;
        case BinaryExpression::BinaryOperator::kEqual:
            stream_ << " == ";
            break;
        case BinaryExpression::BinaryOperator::kNotEqual:
            stream_ << " != ";
            break;
        case BinaryExpression::BinaryOperator::kAnd:
            stream_ << " && ";
            break;
        case BinaryExpression::BinaryOperator::kOr:
            stream_ << " || ";
            break;
    }
    expression->GetRightExpression()->Accept(this);
    stream_ << ")";
}

void PrintVisitor::Visit(ConditionalExpression* expression) {
    stream_ << "(";
    expression->GetCondition()->Accept(this);
    stream_ << " ? ";
    expression->GetLeftExpression()->Accept(this);
    stream_ << " : ";
    expression->GetRightExpression()->Accept(this);
    stream_ << ")";
}

void PrintVisitor::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    stream_ << " = ";
    expression->GetRightExpression()->Accept(this);
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

void PrintVisitor::Visit(ExpressionStatement* statement) {
    PrintTabs();
    stream_ << "ExpressionStatement: ";
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
    stream_ << std::endl;
}

void PrintVisitor::Visit(SelectionStatement* statement) {
    PrintTabs();
    stream_ << "SelectionStatement: if ";
    statement->GetCondition()->Accept(this);
    stream_ << std::endl;

    number_of_tabs_++;
    stream_ << "then: " << std::endl;
    number_of_tabs_++;
    statement->GetThenStatement()->Accept(this);
    number_of_tabs_--;
    if (statement->HasElseStatement()) {
        stream_ << "else: " << std::endl;
        number_of_tabs_++;
        statement->GetElseStatement()->Accept(this);
        number_of_tabs_--;
    }
    number_of_tabs_--;
}

void PrintVisitor::Visit(JumpStatement* statement) {
    PrintTabs();
    stream_ << "JumpStatement: ";
    switch (statement->GetType()) {
        case JumpStatement::JumpType::kBreak:
            stream_ << "break";
            break;
        case JumpStatement::JumpType::kContinue:
            stream_ << "continue";
            break;
    }
    stream_ << std::endl;
}

void PrintVisitor::Visit(WhileStatement* statement) {
    PrintTabs();
    stream_ << "WhileStatement: ";
    switch (statement->GetType()) {
        case WhileStatement::LoopType::kDoWhile:
            stream_ << "do" << std::endl;
            number_of_tabs_++;
            statement->GetBody()->Accept(this);
            number_of_tabs_--;
            PrintTabs();
            stream_ << "while (";
            statement->GetCondition()->Accept(this);
            stream_ << ")" << std::endl;
            break;
        case WhileStatement::LoopType::kWhile:
            stream_ << "while (";
            statement->GetCondition()->Accept(this);
            stream_ << ")" << std::endl;
            number_of_tabs_++;
            statement->GetBody()->Accept(this);
            number_of_tabs_--;
            break;
    }
}

void PrintVisitor::Visit(ForStatement* statement) {
    PrintTabs();
    stream_ << "ForStatement: for (";
    statement->GetInit()->Accept(this);
    stream_ << "; ";
    statement->GetCondition()->Accept(this);
    stream_ << "; ";
    statement->GetIncrement()->Accept(this);
    stream_ << ")" << std::endl;
    number_of_tabs_++;
    statement->GetBody()->Accept(this);
    number_of_tabs_--;
}

void PrintVisitor::PrintTabs() const {
    for (int i = 0; i < number_of_tabs_; ++i) {
        stream_ << '\t';
    }
}