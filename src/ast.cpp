#include "../include/ast.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ast {

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool {
    return kind == BinOpKind::Add || kind == BinOpKind::Sub;
}

[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool {
    return kind == BinOpKind::Eq || kind == BinOpKind::Gt || kind == BinOpKind::Lt ||
           kind == BinOpKind::Neq;
}

[[nodiscard]] auto MoveAstNode::toString() const -> std::string {
    return lhs.toString() + " = " + rhs.toString();
}

}  // namespace ast