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
    std::variant<std::shared_ptr<MoveAstNode>, std::shared_ptr<ReturnAstNode>,
                 std::shared_ptr<IfNode>, std::shared_ptr<JumpAstNode>,
                 std::shared_ptr<FunctionCallAstNode>, std::shared_ptr<ForLoopAstNode>,
                 std::shared_ptr<DerefWriteAstNode>, std::shared_ptr<ConstIntAstNode>,
                 std::shared_ptr<VariableAstNode>, std::shared_ptr<DerefReadAstNode>,
                 std::shared_ptr<AddrAstNode>, std::shared_ptr<BinaryOpAstNode>>
        node;

    Stmt(std::shared_ptr<IfNode> p_node);
    Stmt(std::shared_ptr<MoveAstNode> p_node);
    Stmt(std::shared_ptr<DerefWriteAstNode> p_node);
    Stmt(std::shared_ptr<ReturnAstNode> p_node);
    Stmt(std::shared_ptr<ForLoopAstNode> p_node);
    Stmt(std::shared_ptr<JumpAstNode> p_node);
    Stmt(std::shared_ptr<FunctionCallAstNode> p_node);
    Stmt(std::shared_ptr<ConstIntAstNode> p_node);
    Stmt(std::shared_ptr<VariableAstNode> p_node);
    Stmt(std::shared_ptr<DerefReadAstNode> p_node);
    Stmt(std::shared_ptr<AddrAstNode> p_node);
    Stmt(std::shared_ptr<BinaryOpAstNode> p_node);

    [[nodiscard]] const std::string get_variable_name() const;
    [[nodiscard]] const ast::DataType get_variable_type() const;

    [[nodiscard]] auto is_variable_ast_node() const -> bool;
    [[nodiscard]] auto is_deref_write() const -> bool;

    [[nodiscard]] auto get_variable_ast_node() -> std::shared_ptr<VariableAstNode> ; 

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

}  // namespace ast