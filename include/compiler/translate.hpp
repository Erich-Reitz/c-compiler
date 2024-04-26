#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../ast/ast.hpp"
#include "../parser/st.hpp"
namespace ast {

struct Ctx {
    unsigned long counter = 0;
    bool __lvalueContext = false;
    std::unordered_map<std::string, std::shared_ptr<VariableAstNode>> local_variables;

    void set_lvalueContext(std::string why, bool value) { __lvalueContext = value; }
};

[[nodiscard]] auto translate(const st::Expression& expr, Ctx& ctx) -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::ExpressionStatement>& stmt, Ctx& ctx)
    -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::SelectionStatement>& stmt, Ctx& ctx)
    -> ast::Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::UnaryExpression>& expr, Ctx& ctx) -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::ForStatement>& stmt, Ctx& ctx) -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::AssignmentExpression>& expr, Ctx& ctx)
    -> Stmt;
[[nodiscard]] auto translate(st::CompoundStatement& stmts, Ctx& ctx) -> std::vector<BodyNode>;
[[nodiscard]] auto translate(const std::unique_ptr<st::ReturnStatement>& stmt, Ctx& ctx) -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::AdditiveExpression>& expr, Ctx& ctx) -> Stmt;
[[nodiscard]] auto translate(const std::unique_ptr<st::PrimaryExpression>& expr, Ctx& ctx) -> Stmt;

[[nodiscard]] auto translateStatement(st::Statement& stmt, Ctx& ctx) -> ast::Stmt;
[[nodiscard]] auto translate(st::FuncDef* fd, Ctx& ctx) -> std::shared_ptr<FrameAstNode>;

[[nodiscard]] auto translate(const std::unique_ptr<st::FunctionCallExpression>& expr, Ctx& ctx)
    -> Stmt;

[[nodiscard]] auto translate(const st::Declaration& decl, Ctx& ctx) -> std::shared_ptr<MoveAstNode>;
[[nodiscard]] auto translate(const st::ExternalDeclaration& node, Ctx& ctx) -> ast::TopLevelNode;
[[nodiscard]] std::vector<TopLevelNode> translate(const st::Program& program);
}  // namespace ast