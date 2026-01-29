#include "include/visitors/print_visitor.h"

#include "include/ast/expressions.h"

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
    stream_ << expression->ToString();
}

void PrintVisitor::Visit(UnaryExpression* expression) {
    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::Minus:
            stream_ << "-";
            break;
        case UnaryExpression::UnaryOperator::BinaryNot:
            stream_ << "~";
            break;
        case UnaryExpression::UnaryOperator::Not:
            stream_ << "!";
            break;
        case UnaryExpression::UnaryOperator::Plus:
            stream_ << "+";
            break;
    }
    expression->GetExpression()->Accept(this);
}

void PrintVisitor::Visit(BinaryExpression* expression) {
    stream_ << "(";
    expression->GetLeftExpression()->Accept(this);
    switch (expression->GetOp()) {
        case BinaryExpression::BinaryOperator::Plus:
            stream_ << " + ";
            break;
        case BinaryExpression::BinaryOperator::Minus:
            stream_ << " - ";
            break;
        case BinaryExpression::BinaryOperator::Mul:
            stream_ << " * ";
            break;
        case BinaryExpression::BinaryOperator::Div:
            stream_ << " / ";
            break;
        case BinaryExpression::BinaryOperator::Mod:
            stream_ << " % ";
            break;
        case BinaryExpression::BinaryOperator::Less:
            stream_ << " < ";
            break;
        case BinaryExpression::BinaryOperator::Greater:
            stream_ << " > ";
            break;
        case BinaryExpression::BinaryOperator::LessEqual:
            stream_ << " <= ";
            break;
        case BinaryExpression::BinaryOperator::GreaterEqual:
            stream_ << " >= ";
            break;
        case BinaryExpression::BinaryOperator::Equal:
            stream_ << " == ";
            break;
        case BinaryExpression::BinaryOperator::NotEqual:
            stream_ << " != ";
            break;
        case BinaryExpression::BinaryOperator::And:
            stream_ << " && ";
            break;
        case BinaryExpression::BinaryOperator::Or:
            stream_ << " || ";
            break;
        case BinaryExpression::BinaryOperator::BitwiseAnd:
            stream_ << " & ";
            break;
        case BinaryExpression::BinaryOperator::BitwiseXor:
            stream_ << " ^ ";
            break;
        case BinaryExpression::BinaryOperator::BitwiseOr:
            stream_ << " | ";
            break;
        case BinaryExpression::BinaryOperator::LeftShift:
            stream_ << " << ";
            break;
        case BinaryExpression::BinaryOperator::RightShift:
            stream_ << " >> ";
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

void PrintVisitor::Visit(CastExpression* expression) {
    stream_ << "(";
    expression->GetType()->Accept(this);
    stream_ << ")";
    expression->GetExpression()->Accept(this);
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
        case JumpStatement::JumpType::Break:
            stream_ << "break";
            break;
        case JumpStatement::JumpType::Continue:
            stream_ << "continue";
            break;
    }
    stream_ << std::endl;
}

void PrintVisitor::Visit(WhileStatement* statement) {
    PrintTabs();
    stream_ << "WhileStatement: ";
    switch (statement->GetType()) {
        case WhileStatement::LoopType::DoWhile:
            stream_ << "do ";
            statement->GetBody()->Accept(this);
            stream_ << " while (";
            statement->GetCondition()->Accept(this);
            stream_ << ")";
            break;
        case WhileStatement::LoopType::While:
            stream_ << "while (";
            statement->GetCondition()->Accept(this);
            stream_ << ") ";
            statement->GetBody()->Accept(this);
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

void PrintVisitor::Visit(ParameterDeclaration* declaration) {
    declaration->GetType()->Accept(this);
    stream_ << " ";
    declaration->GetDeclarator()->Accept(this);
}

void PrintVisitor::Visit(ParameterList* list) {
    bool first = true;
    for (auto& param : list->GetParameters()) {
        if (!first) {
            stream_ << ", ";
        }
        param->Accept(this);
        first = false;
    }
}

void PrintVisitor::Visit(FunctionCallExpression* expression) {
    expression->GetFunction()->Accept(this);
    stream_ << "(";
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
    stream_ << ")";
}

void PrintVisitor::Visit(ArgumentExpressionList* list) {
    bool first = true;
    for (auto& arg : list->GetArguments()) {
        if (!first) {
            stream_ << ", ";
        }
        arg->Accept(this);
        first = false;
    }
}

void PrintVisitor::Visit(IdentifierDeclarator* declarator) {
    stream_ << declarator->GetId();
    if (declarator->HasInitializer()) {
        stream_ << " = ";
        declarator->GetInitializer()->Accept(this);
    }
}

void PrintVisitor::Visit(FunctionDeclarator* declarator) {
    stream_ << declarator->GetId();
    stream_ << "(";
    if (declarator->HasParameters()) {
        declarator->GetParameters()->Accept(this);
    }
    stream_ << ")";
}

void PrintVisitor::PrintTabs() const {
    for (int i = 0; i < number_of_tabs_; ++i) {
        stream_ << '\t';
    }
}