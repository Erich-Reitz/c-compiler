#include "../../include/ast/ast.hpp"

namespace ast {

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool {
    return kind == BinOpKind::Add || kind == BinOpKind::Sub;
}

[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool {
    return kind == BinOpKind::Eq || kind == BinOpKind::Gt || kind == BinOpKind::Lt ||
           kind == BinOpKind::Neq;
}

[[nodiscard]] auto MoveAstNode::toString() const -> std::string {
    if (rhs.has_value()) {
        return lhs.toString() + " = " + rhs.value().toString();
    }

    return lhs.toString() + " = " + ";";
}

}  // namespace ast