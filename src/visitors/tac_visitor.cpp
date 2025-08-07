#include "include/visitors/tac_visitor.h"

#include <iostream>
#include <sstream>

TACInstruction::TACInstruction(OpCode op, const std::string& dst, const std::string& lhs,
                               const std::string& rhs)
    : op_(op), dst_(dst), lhs_(lhs), rhs_(rhs) {}

TACInstruction::TACInstruction(OpCode op, const std::string& dst, const std::string& lhs)
    : op_(op), dst_(dst), lhs_(lhs), label_(dst) {}

TACInstruction::TACInstruction(OpCode op, const std::string& label)
    : op_(op), label_(label) {}

TACInstruction::TACInstruction(OpCode op) : op_(op) {}

std::string TACInstruction::ToString() const {
    std::ostringstream out;

    auto OpToStr = [](OpCode op) -> std::string {
        switch (op) {
            case OpCode::Label:
                return "label";
            case OpCode::Global:
                return "global";
            case OpCode::Return:
                return "return";
            case OpCode::Assign:
                return "=";
            case OpCode::Add:
                return "+";
            case OpCode::Sub:
                return "-";
            case OpCode::Mul:
                return "*";
            case OpCode::Div:
                return "/";
            case OpCode::Mod:
                return "%";
            case OpCode::Not:
                return "!";
            case OpCode::Plus:
                return "+";
            case OpCode::Minus:
                return "-";
            case OpCode::BinaryNot:
                return "~";
            case OpCode::Less:
                return "<";
            case OpCode::LessEqual:
                return "<=";
            case OpCode::Greater:
                return ">";
            case OpCode::GreaterEqual:
                return ">=";
            case OpCode::Equal:
                return "==";
            case OpCode::NotEqual:
                return "!=";
            case OpCode::If:
                return "if";
            case OpCode::IfFalse:
                return "iffalse";
            case OpCode::GoTo:
                return "goto";
            case OpCode::BitwiseAnd:
                return "&";
            case OpCode::BitwiseXor:
                return "^";
            case OpCode::BitwiseOr:
                return "|";
            case OpCode::LeftShift:
                return "<<";
            case OpCode::RightShift:
                return ">>";
        }
        return "unknown";
    };

    switch (op_) {
        case OpCode::Label:
            out << label_ << ":";
            break;
        case OpCode::Global:
            out << ".globl " << label_;
            break;
        case OpCode::Return:
            if (!label_.empty())
                out << "return " << label_;
            else
                out << "return";
            break;
        case OpCode::Assign:
            out << dst_ << " = " << lhs_;
            break;
        case OpCode::Add:
        case OpCode::Sub:
        case OpCode::Mul:
        case OpCode::Div:
        case OpCode::Mod:
        case OpCode::Less:
        case OpCode::LessEqual:
        case OpCode::Greater:
        case OpCode::GreaterEqual:
        case OpCode::Equal:
        case OpCode::NotEqual:
        case OpCode::BitwiseAnd:
        case OpCode::BitwiseXor:
        case OpCode::BitwiseOr:
        case OpCode::LeftShift:
        case OpCode::RightShift:
            out << dst_ << " = " << lhs_ << " " << OpToStr(op_) << " " << rhs_;
            break;
        case OpCode::Not:
        case OpCode::BinaryNot:
        case OpCode::Plus:
        case OpCode::Minus:
            out << dst_ << " = " << OpToStr(op_) << lhs_;
            break;
        case OpCode::If:
            out << "if " << lhs_ << " goto " << label_;
            break;
        case OpCode::IfFalse:
            out << "iffalse " << lhs_ << " goto " << label_;
            break;
        case OpCode::GoTo:
            out << "goto " << label_;
            break;
    }

    return out.str();
}

TACInstruction::OpCode TACInstruction::GetOp() const { return op_; }

const std::string& TACInstruction::GetDst() const { return dst_; }

const std::string& TACInstruction::GetLhs() const { return lhs_; }

const std::string& TACInstruction::GetRhs() const { return rhs_; }

const std::string& TACInstruction::GetLabel() const { return label_; }

///////////////////////////////////////////////

TACVisitor::TACVisitor() {}

void TACVisitor::Visit(TranslationUnit* translation_unit) {
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void TACVisitor::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void TACVisitor::Visit(FunctionDefinition* function) {
    std::string name = "_" + function->GetDeclarator()->GetId();
    instructions_.emplace_back(TACInstruction::OpCode::Global, name);
    instructions_.emplace_back(TACInstruction::OpCode::Label, name);
    function->GetBody()->Accept(this);
}

void TACVisitor::Visit(TypeSpecification* type) {}

void TACVisitor::Visit(Declarator* declarator) {}

void TACVisitor::Visit(InitDeclarator* declarator) {
    std::string dst = declarator->GetDeclarator()->GetId();
    std::string src = "0";
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
        src = GetTop();
    }
    instructions_.emplace_back(TACInstruction::OpCode::Assign, dst, src);
}

void TACVisitor::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void TACVisitor::Visit(Expression* expression) { stack_.push("1"); }

void TACVisitor::Visit(IdExpression* expression) { stack_.push(expression->GetId()); }

void TACVisitor::Visit(PrimaryExpression* expression) {
    stack_.push(std::to_string(expression->GetValue()));
}

void TACVisitor::Visit(UnaryExpression* expression) {
    std::string variable_name = GetTemporaryName();
    expression->GetExpression()->Accept(this);
    std::string src = GetTop();
    TACInstruction::OpCode op_code;

    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::Plus:
            op_code = TACInstruction::OpCode::Plus;
            break;
        case UnaryExpression::UnaryOperator::Minus:
            op_code = TACInstruction::OpCode::Minus;
            break;
        case UnaryExpression::UnaryOperator::Not:
            op_code = TACInstruction::OpCode::Not;
            break;
        case UnaryExpression::UnaryOperator::BinaryNot:
            op_code = TACInstruction::OpCode::BinaryNot;
            break;
    }
    instructions_.emplace_back(op_code, variable_name, src);
    stack_.push(variable_name);
}

void TACVisitor::Visit(BinaryExpression* expression) {
    BinaryExpression::BinaryOperator op = expression->GetOp();
    if (op == BinaryExpression::BinaryOperator::Or) {
        ProcessBinaryOr(expression);
        return;
    }

    if (op == BinaryExpression::BinaryOperator::And) {
        ProcessBinaryAnd(expression);
        return;
    }

    expression->GetLeftExpression()->Accept(this);
    std::string lhs = GetTop();
    expression->GetRightExpression()->Accept(this);
    std::string rhs = GetTop();

    std::string variable_name = GetTemporaryName();
    TACInstruction::OpCode op_code;
    switch (op) {
        case BinaryExpression::BinaryOperator::Plus:
            op_code = TACInstruction::OpCode::Add;
            break;
        case BinaryExpression::BinaryOperator::Minus:
            op_code = TACInstruction::OpCode::Sub;
            break;
        case BinaryExpression::BinaryOperator::Mul:
            op_code = TACInstruction::OpCode::Mul;
            break;
        case BinaryExpression::BinaryOperator::Div:
            op_code = TACInstruction::OpCode::Div;
            break;
        case BinaryExpression::BinaryOperator::Mod:
            op_code = TACInstruction::OpCode::Mod;
            break;
        case BinaryExpression::BinaryOperator::Less:
            op_code = TACInstruction::OpCode::Less;
            break;
        case BinaryExpression::BinaryOperator::LessEqual:
            op_code = TACInstruction::OpCode::LessEqual;
            break;
        case BinaryExpression::BinaryOperator::Greater:
            op_code = TACInstruction::OpCode::Greater;
            break;
        case BinaryExpression::BinaryOperator::GreaterEqual:
            op_code = TACInstruction::OpCode::GreaterEqual;
            break;
        case BinaryExpression::BinaryOperator::Equal:
            op_code = TACInstruction::OpCode::Equal;
            break;
        case BinaryExpression::BinaryOperator::NotEqual:
            op_code = TACInstruction::OpCode::NotEqual;
            break;
        case BinaryExpression::BinaryOperator::BitwiseAnd:
            op_code = TACInstruction::OpCode::BitwiseAnd;
            break;
        case BinaryExpression::BinaryOperator::BitwiseXor:
            op_code = TACInstruction::OpCode::BitwiseXor;
            break;
        case BinaryExpression::BinaryOperator::BitwiseOr:
            op_code = TACInstruction::OpCode::BitwiseOr;
            break;
        case BinaryExpression::BinaryOperator::LeftShift:
            op_code = TACInstruction::OpCode::LeftShift;
            break;
        case BinaryExpression::BinaryOperator::RightShift:
            op_code = TACInstruction::OpCode::RightShift;
            break;
        default:
            break;
    }
    instructions_.emplace_back(op_code, variable_name, lhs, rhs);
    stack_.push(variable_name);
}

void TACVisitor::Visit(ConditionalExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_else = "label_else_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = GetTemporaryName();

    expression->GetCondition()->Accept(this);
    std::string cond = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_else, cond);

    expression->GetLeftExpression()->Accept(this);
    std::string value = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, value);
    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_end);

    instructions_.emplace_back(TACInstruction::OpCode::Label, label_else);
    expression->GetRightExpression()->Accept(this);
    value = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, value);

    instructions_.emplace_back(TACInstruction::OpCode::Label, label_end);
    stack_.push(variable_name);
}

void TACVisitor::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    std::string dst = GetTop();

    expression->GetRightExpression()->Accept(this);
    std::string src = GetTop();

    instructions_.emplace_back(TACInstruction::OpCode::Assign, dst, src);
    stack_.push(dst);
}

void TACVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void TACVisitor::Visit(ReturnStatement* statement) {
    std::string value;
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
        value = GetTop();
    }
    instructions_.emplace_back(TACInstruction::OpCode::Return, value);
}

void TACVisitor::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void TACVisitor::Visit(SelectionStatement* statement) {
    std::string label_id = GetUniqueLabelId();
    std::string label_else = "label_else_" + label_id;
    std::string label_end = "label_end_" + label_id;

    statement->GetCondition()->Accept(this);
    std::string cond = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_else, cond);
    statement->GetThenStatement()->Accept(this);
    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_end);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_else);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_end);
}

void TACVisitor::Visit(JumpStatement* statement) {
    std::string label = statement->GetLabel() + "_";
    if (statement->GetType() == JumpStatement::JumpType::Break) {
        label += "break";
    } else {
        label += "continue";
    }

    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label);
}

void TACVisitor::Visit(WhileStatement* statement) {
    std::string label_break = statement->GetLabel() + "_break";
    std::string label_continue = statement->GetLabel() + "_continue";
    std::string label_start = statement->GetLabel() + "_start";

    if (statement->GetType() == WhileStatement::LoopType::While) {
        instructions_.emplace_back(TACInstruction::OpCode::Label, label_continue);
        statement->GetCondition()->Accept(this);
        std::string cond = GetTop();
        instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_break, cond);
        statement->GetBody()->Accept(this);
        instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_continue);
        instructions_.emplace_back(TACInstruction::OpCode::Label, label_break);
    } else {
        instructions_.emplace_back(TACInstruction::OpCode::Label, label_start);
        statement->GetBody()->Accept(this);
        instructions_.emplace_back(TACInstruction::OpCode::Label, label_continue);
        statement->GetCondition()->Accept(this);
        std::string cond = GetTop();
        instructions_.emplace_back(TACInstruction::OpCode::If, label_start, cond);
        instructions_.emplace_back(TACInstruction::OpCode::Label, label_break);
    }
}

void TACVisitor::Visit(ForStatement* statement) {
    std::string label_break = statement->GetLabel() + "_break";
    std::string label_continue = statement->GetLabel() + "_continue";
    std::string label_start = statement->GetLabel() + "_start";

    statement->GetInit()->Accept(this);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_start);
    statement->GetCondition()->Accept(this);
    std::string cond = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_break, cond);
    statement->GetBody()->Accept(this);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_continue);
    statement->GetIncrement()->Accept(this);
    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_start);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_break);
}

std::string TACVisitor::GetTemporaryName() {
    return "temp.." + std::to_string(temp_count_++);
}

std::string TACVisitor::GetUniqueLabelId() { return std::to_string(label_id_++); }

std::string TACVisitor::GetTop() {
    if (stack_.empty()) {
        throw std::runtime_error("stack underflow in TACVisitor::GetTop()");
    }

    std::string value = stack_.top();
    stack_.pop();
    return value;
}

void TACVisitor::ProcessBinaryOr(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_true = "label_true_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = GetTemporaryName();

    expression->GetLeftExpression()->Accept(this);
    std::string lhs = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::If, label_true, lhs);

    expression->GetRightExpression()->Accept(this);
    std::string rhs = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::If, label_true, rhs);

    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, "0");
    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_end);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_true);
    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, "1");
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_end);

    stack_.push(variable_name);
}

void TACVisitor::ProcessBinaryAnd(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_false = "label_false_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = GetTemporaryName();

    expression->GetLeftExpression()->Accept(this);
    std::string lhs = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_false, lhs);

    expression->GetRightExpression()->Accept(this);
    std::string rhs = GetTop();
    instructions_.emplace_back(TACInstruction::OpCode::IfFalse, label_false, rhs);

    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, "1");
    instructions_.emplace_back(TACInstruction::OpCode::GoTo, label_end);
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_false);
    instructions_.emplace_back(TACInstruction::OpCode::Assign, variable_name, "0");
    instructions_.emplace_back(TACInstruction::OpCode::Label, label_end);
    stack_.push(variable_name);
}

void TACVisitor::PrintTACInstructions(std::ostream& out) const {
    for (const auto& instr : instructions_) {
        out << instr.ToString() << '\n';
    }
}

std::vector<TACInstruction> TACVisitor::GetTACInstructions() const {
    return instructions_;
}

void TACVisitor::Visit(FunctionDeclarator* declarator) {
    // TODO: Implement function declarator TAC generation
}

void TACVisitor::Visit(IdentifierDeclarator* declarator) {
    // TODO: Implement identifier declarator TAC generation
}

void TACVisitor::Visit(ParameterDeclaration* declaration) {
    // TODO: Implement parameter declaration TAC generation
}

void TACVisitor::Visit(ParameterList* list) {
    // TODO: Implement parameter list TAC generation
}

void TACVisitor::Visit(FunctionCallExpression* expression) {
    // TODO: Implement function call TAC generation
}

void TACVisitor::Visit(ArgumentExpressionList* list) {
    // TODO: Implement argument list TAC generation
}