#include "../../include/parser/st.hpp"

namespace st {
AssignmentExpression::AssignmentExpression(Expression p_lhs, Expression p_rhs)
    : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)) {}

AdditiveExpression::AdditiveExpression(Expression p_lhs, Expression p_rhs,
                                       AdditiveExpressionType p_type)
    : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)), type(p_type) {}

MultiplicativeExpression::MultiplicativeExpression(Expression p_lhs, Expression p_rhs,
                                                   MultiplicativeExpressionType p_type)
    : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)), type(p_type) {}

SelectionStatement::SelectionStatement(Expression p_cond, std::shared_ptr<CompoundStatement> p_then,
                                       std::shared_ptr<CompoundStatement> p_else)
    : cond(std::move(p_cond)), then(std::move(p_then)), else_(std::move(p_else)) {}

Statement::Statement(
    std::variant<std::shared_ptr<ExpressionStatement>, std::shared_ptr<ReturnStatement>,
                 std::shared_ptr<SelectionStatement>, std::shared_ptr<ForStatement>>
        p_stmt)
    : stmt(std::move(p_stmt)) {}

Statement::Statement(std::shared_ptr<ExpressionStatement> p_stmt) : stmt(std::move(p_stmt)) {}
Statement::Statement(std::shared_ptr<ReturnStatement> p_stmt) : stmt(std::move(p_stmt)) {}
Statement::Statement(std::shared_ptr<SelectionStatement> p_stmt) : stmt(std::move(p_stmt)) {}

BlockItem::BlockItem(std::variant<Declaration, Statement> p_item) : item(std::move(p_item)) {}
BlockItem::BlockItem(Declaration p_item) : item(std::move(p_item)) {}
BlockItem::BlockItem(Statement p_item) : item(std::move(p_item)) {}

UnaryExpression::UnaryExpression(UnaryExpressionType _type, Expression p_expr)
    : type(_type), expr(std::move(p_expr)) {}

FunctionCallExpression::FunctionCallExpression(std::string p_name, std::vector<Expression> p_args)
    : name(std::move(p_name)), args(std::move(p_args)) {}

ForStatement::ForStatement(ForDeclaration p_init, std::optional<Expression> p_cond,
                           std::optional<Expression> p_inc,
                           std::shared_ptr<CompoundStatement> p_body)
    :

      init(std::move(p_init)),
      cond(std::move(p_cond)),
      inc(std::move(p_inc)),

      body(std::move(p_body)) {}

ArrayAccessExpression::ArrayAccessExpression(std::string p_name, Expression p_index)
    : name(std::move(p_name)), index(std::move(p_index)) {}

std::ostream& ForStatement::print(std::ostream& os) const {
    os << "Forstatement(";
    init.print(os);
    if (cond.has_value()) {
        os << cond.value();
    }
    if (inc.has_value()) {
        os << inc.value();
    }
    body->print(os);
    return os;
}

}  // namespace st
