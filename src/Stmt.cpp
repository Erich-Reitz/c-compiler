#include "../include/Stmt.hpp"

#include "../include/ast.hpp"

namespace ast {
Stmt::Stmt(std::unique_ptr<IfNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<MoveAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<DerefWriteAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<ReturnAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<ForLoopAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<JumpAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<FunctionCallAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<ConstIntAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<VariableAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<DerefReadAstNode> p_node) : node(std::move(p_node)) {}
Stmt::Stmt(std::unique_ptr<AddrAstNode> p_node) : node(std::move(p_node)) {}

Stmt::Stmt(std::unique_ptr<BinaryOpAstNode> p_node) : node(std::move(p_node)) {}

[[nodiscard]] auto Stmt::is_variable_ast_node() const -> bool {
    return std::holds_alternative<std::unique_ptr<VariableAstNode>>(node);
}
[[nodiscard]] auto Stmt::is_deref_write() const -> bool {
    return std::holds_alternative<std::unique_ptr<DerefWriteAstNode>>(node);
}

const std::string Stmt::get_variable_name() const {
    if (std::holds_alternative<std::unique_ptr<VariableAstNode>>(node)) {
        return std::get<std::unique_ptr<VariableAstNode>>(node)->name;
    }
    if (std::holds_alternative<std::unique_ptr<AddrAstNode>>(node)) {
        return std::get<std::unique_ptr<AddrAstNode>>(node)->expr.get_variable_name();
    }
    if (std::holds_alternative<std::unique_ptr<DerefWriteAstNode>>(node)) {
        return std::get<std::unique_ptr<DerefWriteAstNode>>(node)->expr.get_variable_name();
    }

    throw std::runtime_error("get_variable_name() not implemented");
}

const ast::DataType Stmt::get_variable_type() const {
    if (std::holds_alternative<std::unique_ptr<VariableAstNode>>(node)) {
        return std::get<std::unique_ptr<VariableAstNode>>(node)->type;
    }

    throw std::runtime_error("get_variable_type() not implemented");
}

[[nodiscard]] std::string Stmt::toString() const {
    return std::visit([](const auto& node) { return node->toString(); }, node);
}

[[nodiscard]] auto BodyNode::is_stmt() const -> bool { return std::holds_alternative<Stmt>(node); }

[[nodiscard]] auto BodyNode::is_move() const -> bool {
    return std::holds_alternative<std::unique_ptr<MoveAstNode>>(node);
}

[[nodiscard]] auto BodyNode::get_stmt() -> Stmt { return std::move(std::get<Stmt>(node)); }

[[nodiscard]] auto BodyNode::get_move() -> std::unique_ptr<MoveAstNode> {
    return std::move(std::get<std::unique_ptr<MoveAstNode>>(node));
}

[[nodiscard]] auto BodyNode::toString() const -> std::string {
    if (is_stmt()) {
        return std::get<Stmt>(node).toString();
    }
    return std::get<std::unique_ptr<MoveAstNode>>(node)->toString();
}

}  // namespace ast