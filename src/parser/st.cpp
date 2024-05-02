#include "../../include/parser/st.hpp"

namespace st {
AssignmentExpression::AssignmentExpression(Expression p_lhs, Expression p_rhs)
    : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)) {}

AdditiveExpression::AdditiveExpression(Expression lhs, Expression rhs, AdditiveExpressionType _type)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), type(_type) {}

SelectionStatement::SelectionStatement(Expression cond, std::shared_ptr<CompoundStatement> then,
                                       std::shared_ptr<CompoundStatement> else_)
    : cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)) {}

Statement::Statement(
    std::variant<std::shared_ptr<ExpressionStatement>, std::shared_ptr<ReturnStatement>,
                 std::shared_ptr<SelectionStatement>, std::shared_ptr<ForStatement>>
        p_stmt)
    : stmt(std::move(p_stmt)) {}

Statement::Statement(std::shared_ptr<ExpressionStatement> stmt) : stmt(std::move(stmt)) {}
Statement::Statement(std::shared_ptr<ReturnStatement> stmt) : stmt(std::move(stmt)) {}
Statement::Statement(std::shared_ptr<SelectionStatement> stmt) : stmt(std::move(stmt)) {}

BlockItem::BlockItem(std::variant<Declaration, Statement> item) : item(std::move(item)) {}
BlockItem::BlockItem(Declaration item) : item(std::move(item)) {}
BlockItem::BlockItem(Statement item) : item(std::move(item)) {}

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
    os << "for stmt";
    return os;
}

}  // namespace st
