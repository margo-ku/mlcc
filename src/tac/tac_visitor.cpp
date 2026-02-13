#include "include/tac/tac_visitor.h"

#include <iostream>

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/semantic/symbol_table.h"

TACVisitor::TACVisitor(SymbolTable& symbol_table) : symbol_table_(symbol_table) {}

void TACVisitor::Visit(TranslationUnit* translation_unit) {
    const auto& declarations = translation_unit->GetExternalDeclarations();
    for (auto& declaration : declarations) {
        declaration->Accept(this);
    }
}

void TACVisitor::Visit(ItemList* item_list) {
    const auto& items = item_list->GetItems();
    for (auto& item : items) {
        item->Accept(this);
    }
}

void TACVisitor::Visit(FunctionDefinition* function) {
    auto declarator = dynamic_cast<FunctionDeclarator*>(function->GetDeclarator());
    auto params = declarator->GetParameters();

    int param_count = 0;
    if (params) {
        param_count = params->GetParameters().size();
    }

    std::string name = declarator->GetId();

    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    bool is_global = info->linkage != SymbolInfo::LinkageKind::Internal;

    instructions_.emplace_back();
    instructions_.back().push_back(
        TACInstruction::Function(name, param_count, is_global));

    if (params) {
        params->Accept(this);
    }

    function->GetBody()->Accept(this);
}

void TACVisitor::Visit(DeclarationSpecifiers* decl_specs) {}

void TACVisitor::Visit(TypeSpecification* type) {}

void TACVisitor::Visit(Declaration* declaration) {
    auto decl = declaration->GetDeclaration();
    decl->Accept(this);
}

void TACVisitor::Visit(Expression* expression) {
    stack_.push(TACOperand(IntegralConstant(1)));
}

void TACVisitor::Visit(IdExpression* expression) { stack_.push(expression->GetId()); }

void TACVisitor::Visit(PrimaryExpression* expression) {
    stack_.push(TACOperand(expression->GetValue()));
}

void TACVisitor::Visit(UnaryExpression* expression) {
    std::string variable_name = AllocateTemporary(expression->GetTypeRef());
    expression->GetExpression()->Accept(this);
    TACOperand src = GetTop();
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
    instructions_.back().push_back(TACInstruction::Unary(op_code, variable_name, src));
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
    TACOperand lhs = GetTop();

    expression->GetRightExpression()->Accept(this);
    TACOperand rhs = GetTop();

    std::string variable_name = AllocateTemporary(expression->GetTypeRef());

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
        case BinaryExpression::BinaryOperator::And:
        case BinaryExpression::BinaryOperator::Or:
            break;
    }

    instructions_.back().push_back(
        TACInstruction::Binary(op_code, variable_name, lhs, rhs));
    stack_.push(variable_name);
}

void TACVisitor::Visit(ConditionalExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_else = "label_else_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = AllocateTemporary(expression->GetTypeRef());

    expression->GetCondition()->Accept(this);
    TACOperand cond = GetTop();
    instructions_.back().push_back(TACInstruction::IfFalse(label_else, cond));

    expression->GetLeftExpression()->Accept(this);
    TACOperand value = GetTop();
    instructions_.back().push_back(TACInstruction::Assign(variable_name, value));
    instructions_.back().push_back(TACInstruction::GoTo(label_end));

    instructions_.back().push_back(TACInstruction::Label(label_else));
    expression->GetRightExpression()->Accept(this);
    value = GetTop();
    instructions_.back().push_back(TACInstruction::Assign(variable_name, value));

    instructions_.back().push_back(TACInstruction::Label(label_end));
    stack_.push(variable_name);
}

void TACVisitor::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    TACOperand dst = GetTop();

    expression->GetRightExpression()->Accept(this);
    TACOperand src = GetTop();

    instructions_.back().push_back(TACInstruction::Assign(dst, src));
    stack_.push(dst);
}

void TACVisitor::Visit(CastExpression* expression) {
    expression->GetExpression()->Accept(this);
    TACOperand src = GetTop();
    std::string dst = AllocateTemporary(expression->GetTypeRef());
    TypeRef from_type = expression->GetExpression()->GetTypeRef();
    TypeRef to_type = expression->GetTypeRef();
    if (!from_type || !to_type) {
        throw std::runtime_error("invalid types in cast expression");
    }

    if (from_type->Equals(to_type) || from_type->Size() == to_type->Size()) {
        instructions_.back().push_back(TACInstruction::Assign(dst, src));
    } else if (from_type->Size() > to_type->Size()) {
        instructions_.back().push_back(TACInstruction::Truncate(dst, src));
    } else if (from_type->IsSigned()) {
        instructions_.back().push_back(TACInstruction::SignExtend(dst, src));
    } else {
        instructions_.back().push_back(TACInstruction::ZeroExtend(dst, src));
    }
    stack_.push(dst);
}

void TACVisitor::Visit(CompoundStatement* statement) {
    auto body = statement->GetBody();
    body->Accept(this);
}

void TACVisitor::Visit(ReturnStatement* statement) {
    TACOperand value = TACOperand("");
    if (statement->HasExpression()) {
        auto expression = statement->GetExpression();
        expression->Accept(this);
        value = GetTop();
    }
    if (statement->HasExpression()) {
        instructions_.back().push_back(TACInstruction::Return(value));
    } else {
        instructions_.back().push_back(TACInstruction::Return());
    }
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
    TACOperand cond = GetTop();
    instructions_.back().push_back(TACInstruction::IfFalse(label_else, cond));
    statement->GetThenStatement()->Accept(this);
    instructions_.back().push_back(TACInstruction::GoTo(label_end));
    instructions_.back().push_back(TACInstruction::Label(label_else));
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
    instructions_.back().push_back(TACInstruction::Label(label_end));
}

void TACVisitor::Visit(JumpStatement* statement) {
    std::string label = statement->GetLabel() + "_";
    if (statement->GetType() == JumpStatement::JumpType::Break) {
        label += "break";
    } else {
        label += "continue";
    }

    instructions_.back().push_back(TACInstruction::GoTo(label));
}

void TACVisitor::Visit(WhileStatement* statement) {
    std::string label_break = statement->GetLabel() + "_break";
    std::string label_continue = statement->GetLabel() + "_continue";
    std::string label_start = statement->GetLabel() + "_start";

    if (statement->GetType() == WhileStatement::LoopType::While) {
        instructions_.back().push_back(TACInstruction::Label(label_continue));
        statement->GetCondition()->Accept(this);
        TACOperand cond = GetTop();
        instructions_.back().push_back(TACInstruction::IfFalse(label_break, cond));
        statement->GetBody()->Accept(this);
        instructions_.back().push_back(TACInstruction::GoTo(label_continue));
        instructions_.back().push_back(TACInstruction::Label(label_break));
    } else {
        instructions_.back().push_back(TACInstruction::Label(label_start));
        statement->GetBody()->Accept(this);
        instructions_.back().push_back(TACInstruction::Label(label_continue));
        statement->GetCondition()->Accept(this);
        TACOperand cond = GetTop();
        instructions_.back().push_back(TACInstruction::If(label_start, cond));
        instructions_.back().push_back(TACInstruction::Label(label_break));
    }
}

void TACVisitor::Visit(ForStatement* statement) {
    std::string label_break = statement->GetLabel() + "_break";
    std::string label_continue = statement->GetLabel() + "_continue";
    std::string label_start = statement->GetLabel() + "_start";

    statement->GetInit()->Accept(this);
    instructions_.back().push_back(TACInstruction::Label(label_start));
    statement->GetCondition()->Accept(this);
    TACOperand cond = GetTop();
    instructions_.back().push_back(TACInstruction::IfFalse(label_break, cond));
    statement->GetBody()->Accept(this);
    instructions_.back().push_back(TACInstruction::Label(label_continue));
    statement->GetIncrement()->Accept(this);
    instructions_.back().push_back(TACInstruction::GoTo(label_start));
    instructions_.back().push_back(TACInstruction::Label(label_break));
}

void TACVisitor::Visit(FunctionDeclarator* declarator) {}

void TACVisitor::Visit(IdentifierDeclarator* declarator) {
    const std::string dst = declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(dst);
    if (info && info->HasStaticDuration()) {
        return;
    }

    if (!declarator->HasInitializer()) {
        return;
    }

    if (instructions_.empty()) {
        throw std::runtime_error(
            "internal error: automatic declarator outside of function in TACVisitor");
    }

    declarator->GetInitializer()->Accept(this);
    const TACOperand src = GetTop();
    instructions_.back().push_back(TACInstruction::Assign(dst, src));
}

void TACVisitor::Visit(ParameterDeclaration* declaration) {
    std::string param_name = declaration->GetDeclarator()->GetId();
    stack_.push(param_name);
}

void TACVisitor::Visit(ParameterList* list) {
    int index = 0;
    for (auto& param : list->GetParameters()) {
        param->Accept(this);
        std::string param_name = GetTop().AsIdentifier();
        std::string arg_name = "arg.." + std::to_string(index++);
        instructions_.back().push_back(TACInstruction::Assign(param_name, arg_name));
    }
}

void TACVisitor::Visit(FunctionCallExpression* expression) {
    auto function = expression->GetFunction();

    std::string function_name;

    if (auto id_expr = dynamic_cast<IdExpression*>(function)) {
        function_name = id_expr->GetId();
    } else if (auto func_decl = dynamic_cast<FunctionDeclarator*>(function)) {
        function_name = func_decl->GetId();
    }

    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }

    int num_args = 0;
    if (expression->HasArguments()) {
        num_args = expression->GetArguments()->GetArguments().size();
    }

    std::string return_value = AllocateTemporary(expression->GetTypeRef());

    instructions_.back().push_back(
        TACInstruction::Call(return_value, function_name, num_args));

    stack_.push(return_value);
}

void TACVisitor::Visit(ArgumentExpressionList* list) {
    const auto& arguments = list->GetArguments();

    for (size_t index = 0; index < arguments.size(); ++index) {
        arguments[index]->Accept(this);
        TACOperand src = GetTop();
        instructions_.back().push_back(TACInstruction::Param(src));
    }
}

std::string TACVisitor::AllocateTemporary(TypeRef type) {
    std::string name = GetTemporaryName();
    symbol_table_.Register({
        .name = name,
        .original_name = name,
        .type = type,
    });
    return name;
}

std::string TACVisitor::GetTemporaryName() {
    return "temp.." + std::to_string(temp_count_++);
}

std::string TACVisitor::GetUniqueLabelId() { return std::to_string(label_id_++); }

TACOperand TACVisitor::GetTop() {
    if (stack_.empty()) {
        throw std::runtime_error("stack underflow in TACVisitor::GetTop()");
    }

    TACOperand value = stack_.top();
    stack_.pop();
    return value;
}

void TACVisitor::ProcessBinaryOr(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_true = "label_true_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = AllocateTemporary(expression->GetTypeRef());

    expression->GetLeftExpression()->Accept(this);
    TACOperand lhs = GetTop();
    instructions_.back().push_back(TACInstruction::If(label_true, lhs));

    expression->GetRightExpression()->Accept(this);
    TACOperand rhs = GetTop();
    instructions_.back().push_back(TACInstruction::If(label_true, rhs));

    instructions_.back().push_back(
        TACInstruction::Assign(variable_name, TACOperand(IntegralConstant(0))));
    instructions_.back().push_back(TACInstruction::GoTo(label_end));
    instructions_.back().push_back(TACInstruction::Label(label_true));
    instructions_.back().push_back(
        TACInstruction::Assign(variable_name, TACOperand(IntegralConstant(1))));
    instructions_.back().push_back(TACInstruction::Label(label_end));

    stack_.push(variable_name);
}

void TACVisitor::ProcessBinaryAnd(BinaryExpression* expression) {
    std::string label_id = GetUniqueLabelId();
    std::string label_false = "label_false_" + label_id;
    std::string label_end = "label_end_" + label_id;
    std::string variable_name = AllocateTemporary(expression->GetTypeRef());

    expression->GetLeftExpression()->Accept(this);
    TACOperand lhs = GetTop();
    instructions_.back().push_back(TACInstruction::IfFalse(label_false, lhs));

    expression->GetRightExpression()->Accept(this);
    TACOperand rhs = GetTop();
    instructions_.back().push_back(TACInstruction::IfFalse(label_false, rhs));

    instructions_.back().push_back(
        TACInstruction::Assign(variable_name, TACOperand(IntegralConstant(1))));
    instructions_.back().push_back(TACInstruction::GoTo(label_end));
    instructions_.back().push_back(TACInstruction::Label(label_false));
    instructions_.back().push_back(
        TACInstruction::Assign(variable_name, TACOperand(IntegralConstant(0))));
    instructions_.back().push_back(TACInstruction::Label(label_end));
    stack_.push(variable_name);
}

void TACVisitor::PrintTACInstructions(std::ostream& out) const {
    PrintTACInstructions(out, instructions_);
}

void TACVisitor::PrintTACInstructions(
    std::ostream& out, const std::vector<std::vector<TACInstruction>>& instructions) {
    for (const auto& instruction : instructions) {
        for (const auto& instr : instruction) {
            out << instr.ToString() << '\n';
        }
        out << '\n';
    }
}

std::vector<std::vector<TACInstruction>> TACVisitor::GetTACInstructions() const {
    return instructions_;
}

void PrintTACInstructions(std::ostream& out,
                          const std::vector<std::vector<TACInstruction>>& instructions) {
    TACVisitor::PrintTACInstructions(out, instructions);
}

void TACVisitor::AddStaticVariables() {
    for (const auto& [name, info] : symbol_table_.GetAllSymbols()) {
        if (!info.HasStaticDuration()) {
            continue;
        }

        if (info.init_state == SymbolInfo::InitialValue::NoInitializer) {
            continue;
        }

        bool is_global = info.linkage != SymbolInfo::LinkageKind::Internal;
        IntegralConstant initializer =
            (info.init_state == SymbolInfo::InitialValue::Initial)
                ? *info.init_constant
                : IntegralConstant(0);

        instructions_.emplace_back().push_back(TACInstruction::StaticVariable(
            name, initializer, is_global));
    }
}
