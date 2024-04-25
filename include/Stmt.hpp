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
struct VariableAstNode;
struct DerefReadAstNode;
struct AddrAstNode;
struct BinaryOpAstNode;

struct Stmt : public AstNode {
    std::variant<std::unique_ptr<MoveAstNode>, std::unique_ptr<ReturnAstNode>,
                 std::unique_ptr<IfNode>, std::unique_ptr<JumpAstNode>,
                 std::unique_ptr<FunctionCallAstNode>, std::unique_ptr<ForLoopAstNode>,
                 std::unique_ptr<DerefWriteAstNode>, std::unique_ptr<ConstIntAstNode>,
                 std::unique_ptr<VariableAstNode>, std::unique_ptr<DerefReadAstNode>,
                 std::unique_ptr<AddrAstNode>, std::unique_ptr<BinaryOpAstNode>>
        node;

    Stmt(std::unique_ptr<IfNode> p_node);
    Stmt(std::unique_ptr<MoveAstNode> p_node);
    Stmt(std::unique_ptr<DerefWriteAstNode> p_node);
    Stmt(std::unique_ptr<ReturnAstNode> p_node);
    Stmt(std::unique_ptr<ForLoopAstNode> p_node);
    Stmt(std::unique_ptr<JumpAstNode> p_node);
    Stmt(std::unique_ptr<FunctionCallAstNode> p_node);
    Stmt(std::unique_ptr<ConstIntAstNode> p_node);
    Stmt(std::unique_ptr<VariableAstNode> p_node);
    Stmt(std::unique_ptr<DerefReadAstNode> p_node);
    Stmt(std::unique_ptr<AddrAstNode> p_node);
    Stmt(std::unique_ptr<BinaryOpAstNode> p_node);

    [[nodiscard]] const std::string get_variable_name() const;
    [[nodiscard]] const ast::DataType get_variable_type() const;

    [[nodiscard]] auto is_variable_ast_node() const -> bool;
    [[nodiscard]] auto is_deref_write() const -> bool;

    [[nodiscard]] std::string toString() const override;
};

struct BodyNode : public AstNode {
    std::variant<Stmt, std::unique_ptr<MoveAstNode>> node;

    BodyNode(std::unique_ptr<MoveAstNode> p_node) : node(std::move(p_node)) {}
    BodyNode(Stmt p_node) : node(std::move(p_node)) {}

    [[nodiscard]] auto is_stmt() const -> bool;
    [[nodiscard]] auto is_move() const -> bool;
    [[nodiscard]] auto get_stmt() -> Stmt;
    [[nodiscard]] auto get_move() -> std::unique_ptr<MoveAstNode>;

    [[nodiscard]] auto toString() const -> std::string override;
};

}  // namespace ast