#pragma once

#include <string>

namespace ast {

enum class BinOpKind { Add, Sub, Eq, Gt, Lt, Neq };

[[nodiscard]] auto bin_op_to_string(BinOpKind kind) -> std::string;

class AstNode {
   public:
    virtual ~AstNode() = default;
    [[nodiscard]] virtual std::string toString() const = 0;
    virtual const ast::BinOpKind* get_bin_op() const { return nullptr; }
};

}  // namespace ast