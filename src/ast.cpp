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
    return kind == BinOpKind::Eq || kind == BinOpKind::Gt ||
           kind == BinOpKind::Lt || kind == BinOpKind::Neq;
}

std::unique_ptr<Node> makeConstInt(int value) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::ConstInt;
    node->value = value;
    return node;
}

std::unique_ptr<Node> makeNewFunction(
    std::string functionName, std::vector<std::unique_ptr<ast::Node>> body,
    std::vector<FrameParam> params) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Frame;
    node->functionName = functionName;
    node->body = std::move(body);
    node->params = params;
    return node;
}

std::unique_ptr<Node> makeJump(std::string label) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Jump;
    node->jumpToLabelValue = label;
    return node;
}

std::unique_ptr<Node> makeNewReturn(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Return;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewVar(std::string name, DataType type) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Var;
    node->variableName = name;
    node->variableType = type;
    return node;
}

std::unique_ptr<Node> makeNewMove(std::unique_ptr<ast::Node> left,
                                  std::unique_ptr<ast::Node> right) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Move;
    node->lhs = std::move(left);
    node->rhs = std::move(right);
    return node;
}

std::unique_ptr<Node> makeNewForLoop(
    std::unique_ptr<ast::Node> init,
    std::optional<std::unique_ptr<ast::Node>> condition,
    std::optional<std::unique_ptr<ast::Node>> update,
    std::vector<std::unique_ptr<ast::Node>> body) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::ForLoop;
    node->forInit = std::move(init);
    node->forCondition = std::move(condition);
    node->forUpdate = std::move(update);
    node->forBody = std::move(body);
    return node;
}

std::unique_ptr<Node> makeNewMemWrite(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Deref;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewMemRead(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Deref;
    node->expr = std::move(expr);
    node->derefDepth = 1;
    if (node->expr->type == NodeType::Addr) {
        node->type = node->expr->expr->type;
        // hack to copy, assumes that it is a variable. works now for nested
        // stuff
        node->variableName = node->expr->expr->variableName;
    }
    if (node->expr->type == NodeType::Deref) {
        auto result = std::move(node->expr);
        result->derefDepth += 1;
        return result;
    }
    return node;
}

std::unique_ptr<Node> makeNewAddr(std::unique_ptr<ast::Node> expr) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Addr;
    node->expr = std::move(expr);
    return node;
}

std::unique_ptr<Node> makeNewBinOp(std::unique_ptr<ast::Node> lhs,
                                   std::unique_ptr<ast::Node> rhs,
                                   BinOpKind kind) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::BinOp;
    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    node->binOpKind = kind;
    return node;
}

std::unique_ptr<Node> makeNewIfStmt(
    std::unique_ptr<ast::Node> condition,
    std::vector<std::unique_ptr<ast::Node>> then,
    std::vector<std::unique_ptr<ast::Node>> else_) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::If;
    node->condition = std::move(condition);
    node->then = std::move(then);
    node->else_ = std::move(else_);
    return node;
}

std::unique_ptr<Node> makeNewCall(
    std::string name, std::vector<std::unique_ptr<ast::Node>> args) {
    auto node = std::make_unique<Node>();
    node->type = NodeType::Call;
    node->callName = name;
    node->callArgs = std::move(args);
    // TODO: obviously not it
    node->returnType = DataType("int", 4, nullptr);
    return node;
}

}  // namespace ast