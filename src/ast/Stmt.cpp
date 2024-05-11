#include "../../include/ast/ast.hpp"

namespace ast {

auto Stmt::is_variable_ast_node() const -> bool {
    return std::holds_alternative<std::shared_ptr<VariableAstNode>>(node);
}

auto Stmt::get_variable_ast_node() -> std::shared_ptr<VariableAstNode> {
    return std::get<std::shared_ptr<VariableAstNode>>(node);
}

auto Stmt::get_binary_op_node() -> std::shared_ptr<BinaryOpAstNode> {
    return std::get<std::shared_ptr<BinaryOpAstNode>>(node);
}

auto Stmt::is_deref_write() const -> bool {
    return std::holds_alternative<std::shared_ptr<DerefWriteAstNode>>(node);
}

const std::string Stmt::get_variable_name() const {
    if (std::holds_alternative<std::shared_ptr<VariableAstNode>>(node)) {
        return std::get<std::shared_ptr<VariableAstNode>>(node)->name;
    }
    if (std::holds_alternative<std::shared_ptr<AddrAstNode>>(node)) {
        return std::get<std::shared_ptr<AddrAstNode>>(node)->expr.get_variable_name();
    }
    if (std::holds_alternative<std::shared_ptr<DerefWriteAstNode>>(node)) {
        return std::get<std::shared_ptr<DerefWriteAstNode>>(node)->expr.get_variable_name();
    }

    throw std::runtime_error("get_variable_name() not implemented");
}

const ast::DataType Stmt::get_variable_type() const {
    if (std::holds_alternative<std::shared_ptr<VariableAstNode>>(node)) {
        return std::get<std::shared_ptr<VariableAstNode>>(node)->type;
    }

    throw std::runtime_error("get_variable_type() not implemented");
}

std::string Stmt::toString() const {
    return std::visit([](const auto& v_node) { return v_node->toString(); }, node);
}

auto BodyNode::is_stmt() const -> bool { return std::holds_alternative<Stmt>(node); }

auto BodyNode::is_move() const -> bool {
    return std::holds_alternative<std::shared_ptr<MoveAstNode>>(node);
}

auto BodyNode::get_stmt() -> Stmt { return std::move(std::get<Stmt>(node)); }

auto BodyNode::get_move() -> std::shared_ptr<MoveAstNode> {
    return std::move(std::get<std::shared_ptr<MoveAstNode>>(node));
}

auto BodyNode::toString() const -> std::string {
    if (is_stmt()) {
        return std::get<Stmt>(node).toString();
    }
    return std::get<std::shared_ptr<MoveAstNode>>(node)->toString();
}

auto ExprNode::toString() const -> std::string {
    return std::visit([](const auto& v_node) { return v_node->toString(); }, node);
}

const std::string ExprNode::get_variable_name() const {
    if (std::holds_alternative<std::shared_ptr<VariableAstNode>>(node)) {
        return std::get<std::shared_ptr<VariableAstNode>>(node)->name;
    }
    if (std::holds_alternative<std::shared_ptr<AddrAstNode>>(node)) {
        return std::get<std::shared_ptr<AddrAstNode>>(node)->expr.get_variable_name();
    }
    if (std::holds_alternative<std::shared_ptr<DerefWriteAstNode>>(node)) {
        return std::get<std::shared_ptr<DerefWriteAstNode>>(node)->expr.get_variable_name();
    }

    throw std::runtime_error("get_variable_name() not implemented");
}

ast::DataType ExprNode::get_variable_type() const {
    if (std::holds_alternative<std::shared_ptr<VariableAstNode>>(node)) {
        return std::get<std::shared_ptr<VariableAstNode>>(node)->type;
    }

    throw std::runtime_error("get_variable_type() not implemented");
}

auto ExprNode::is_variable_ast_node() const -> bool {
    return std::holds_alternative<std::shared_ptr<VariableAstNode>>(node);
}

auto ExprNode::get_variable_ast_node() -> std::shared_ptr<VariableAstNode> {
    return std::get<std::shared_ptr<VariableAstNode>>(node);
}

auto ExprNode::get_binary_op_node() -> std::shared_ptr<BinaryOpAstNode> {
    return std::get<std::shared_ptr<BinaryOpAstNode>>(node);
}

auto ExprNode::is_deref_write() const -> bool {
    return std::holds_alternative<std::shared_ptr<DerefWriteAstNode>>(node);
}

}  // namespace ast