#include "../../include/ast/ast.hpp"

namespace ast {
[[nodiscard]] auto bin_op_to_string(ast::BinOpKind kind) -> std::string {
    switch (kind) {
        case ast::BinOpKind::Add:
            return "+";
        case ast::BinOpKind::Sub:
            return "-";
        case ast::BinOpKind::Eq:
            return "==";
        case ast::BinOpKind::Gt:
            return ">";
        case ast::BinOpKind::Lt:
            return "<";
        case ast::BinOpKind::Neq:
            return "!=";
    }
    return "bin_op_to_string unkown";
}
}  // namespace ast