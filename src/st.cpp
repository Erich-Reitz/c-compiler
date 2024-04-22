#include "../include/st.hpp"

namespace st {
AssignmentExpression::AssignmentExpression(Expression p_lhs, Expression p_rhs)
    : lhs(std::move(p_lhs)), rhs(std::move(p_rhs)) {}

AdditiveExpression::AdditiveExpression(Expression lhs, Expression rhs,
                                       AdditiveExpressionType _type)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), type(_type) {}

SelectionStatement::SelectionStatement(Expression cond,
                                       std::unique_ptr<CompoundStatement> then,
                                       std::unique_ptr<CompoundStatement> else_)
    : cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)) {}

Statement::Statement(
    std::variant<
        std::unique_ptr<ExpressionStatement>, std::unique_ptr<ReturnStatement>,
        std::unique_ptr<SelectionStatement>, std::unique_ptr<ForStatement>>
        stmt)
    : stmt(std::move(stmt)) {}

Statement::Statement(std::unique_ptr<ExpressionStatement> stmt)
    : stmt(std::move(stmt)) {}
Statement::Statement(std::unique_ptr<ReturnStatement> stmt)
    : stmt(std::move(stmt)) {}
Statement::Statement(std::unique_ptr<SelectionStatement> stmt)
    : stmt(std::move(stmt)) {}

BlockItem::BlockItem(std::variant<Declaration, Statement> item)
    : item(std::move(item)) {}
BlockItem::BlockItem(Declaration item) : item(std::move(item)) {}
BlockItem::BlockItem(Statement item) : item(std::move(item)) {}

UnaryExpression::UnaryExpression(UnaryExpressionType _type, Expression p_expr)
    : type(_type), expr(std::move(p_expr)) {}

FunctionCallExpression::FunctionCallExpression(std::string p_name,
                                               std::vector<Expression> p_args)
    : name(std::move(p_name)), args(std::move(p_args)) {}

ForStatement::ForStatement(ForDeclaration p_init,
                           std::optional<Expression> p_cond,
                           std::optional<Expression> p_inc,
                           std::unique_ptr<CompoundStatement> p_body)
    :

      init(std::move(p_init)),
      cond(std::move(p_cond)),
      inc(std::move(p_inc)),

      body(std::move(p_body)) {}

std::ostream& ForStatement::print(std::ostream& os) const {
    os << "for stmt";
    return os;
}

}  // namespace st
