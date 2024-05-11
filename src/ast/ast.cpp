#include "../../include/ast/ast.hpp"

#include <algorithm>

namespace ast {

inline const std::vector<BinOpKind> arithmetic_operators = {BinOpKind::Add, BinOpKind::Sub,
                                                            BinOpKind::Mul, BinOpKind::Div};

inline const std::vector<BinOpKind> comparison_operators = {BinOpKind::Eq, BinOpKind::Gt,
                                                            BinOpKind::Lt, BinOpKind::Neq};

[[nodiscard]] auto bin_op_to_string(ast::BinOpKind kind) -> std::string {
    switch (kind) {
        case ast::BinOpKind::Add:
            return "+";
        case ast::BinOpKind::Sub:
            return "-";
        case ast::BinOpKind::Mul:
            return "*";
        case ast::BinOpKind::Div:
            return "/";
        case ast::BinOpKind::Eq:
            return "==";
        case ast::BinOpKind::Gt:
            return ">";
        case ast::BinOpKind::Lt:
            return "<";
        case ast::BinOpKind::Neq:
            return "!=";
    }
    return "bin_op_to_string unknown";
}

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool {
    return std::find(arithmetic_operators.begin(), arithmetic_operators.end(), kind) !=
           arithmetic_operators.end();
}

[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool {
    return std::find(comparison_operators.begin(), comparison_operators.end(), kind) !=
           comparison_operators.end();
}

[[nodiscard]] auto MoveAstNode::toString() const -> std::string {
    if (rhs.has_value()) {
        return lhs.toString() + " = " + rhs.value().toString();
    }

    return lhs.toString() + " = " + ";";
}

}  // namespace ast