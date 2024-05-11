#pragma once
#include <memory>
#include <string>
#include <variant>

#include "AstNode.hpp"
#include "DataType.hpp"

namespace ast {
struct MoveAstNode;
struct ReturnAstNode;
struct IfNode;
struct JumpAstNode;
struct FunctionCallAstNode;
struct ForLoopAstNode;
struct DerefWriteAstNode;
struct ConstIntAstNode;
struct ConstFloatNode;
struct VariableAstNode;
struct DerefReadAstNode;
struct AddrAstNode;
struct BinaryOpAstNode;

struct Stmt : public AstNode {
    std::variant<std::shared_ptr<MoveAstNode>, std::shared_ptr<ReturnAstNode>,
                 std::shared_ptr<IfNode>, std::shared_ptr<JumpAstNode>,
                 std::shared_ptr<FunctionCallAstNode>, std::shared_ptr<ForLoopAstNode>,
                 std::shared_ptr<DerefWriteAstNode>, std::shared_ptr<ConstIntAstNode>,
                 std::shared_ptr<VariableAstNode>, std::shared_ptr<DerefReadAstNode>,
                 std::shared_ptr<AddrAstNode>, std::shared_ptr<BinaryOpAstNode>,
                 std::shared_ptr<ConstFloatNode>>
        node;

    template <typename T>
    Stmt(std::shared_ptr<T> p_node) : node(std::move(p_node)) {}

    [[nodiscard]] const std::string get_variable_name() const;
    [[nodiscard]] const ast::DataType get_variable_type() const;

    [[nodiscard]] auto is_variable_ast_node() const -> bool;
    [[nodiscard]] auto is_deref_write() const -> bool;

    [[nodiscard]] auto get_variable_ast_node() -> std::shared_ptr<VariableAstNode>;
    [[nodiscard]] auto get_binary_op_node() -> std::shared_ptr<BinaryOpAstNode>;

    [[nodiscard]] std::string toString() const override;
};

struct BodyNode : public AstNode {
    std::variant<Stmt, std::shared_ptr<MoveAstNode>> node;

    BodyNode(std::shared_ptr<MoveAstNode> p_node) : node(std::move(p_node)) {}
    BodyNode(Stmt p_node) : node(std::move(p_node)) {}

    [[nodiscard]] auto is_stmt() const -> bool;
    [[nodiscard]] auto is_move() const -> bool;
    [[nodiscard]] auto get_stmt() -> Stmt;
    [[nodiscard]] auto get_move() -> std::shared_ptr<MoveAstNode>;

    [[nodiscard]] auto toString() const -> std::string override;
};

struct ExprNode : public AstNode {
    std::variant<std::shared_ptr<ConstIntAstNode>, std::shared_ptr<VariableAstNode>,
                 std::shared_ptr<DerefReadAstNode>, std::shared_ptr<AddrAstNode>,
                 std::shared_ptr<BinaryOpAstNode>, std::shared_ptr<ConstFloatNode>,
                 std::shared_ptr<MoveAstNode>, std::shared_ptr<DerefWriteAstNode>,
                 std::shared_ptr<FunctionCallAstNode>>
        node;

    template <typename T>
    ExprNode(std::shared_ptr<T> p_node) : node(p_node) {}

    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] const std::string get_variable_name() const;
    [[nodiscard]] auto is_variable_ast_node() const -> bool;
    [[nodiscard]] ast::DataType get_variable_type() const;
    [[nodiscard]] auto get_variable_ast_node() -> std::shared_ptr<VariableAstNode>;
    [[nodiscard]] auto get_binary_op_node() -> std::shared_ptr<BinaryOpAstNode>;
    [[nodiscard]] auto is_deref_write() const -> bool;
};

}  // namespace ast