#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "qa_ir.hpp"
#include "qa_x86.hpp"

namespace qa_ir {

struct Frame {
    std::string name;
    std::vector<Operation> instructions;
    int size = 0;
};

struct F_Ctx {
    int temp_counter = 0;
    int label_counter = 0;
    std::map<std::string, std::shared_ptr<ast::VariableAstNode>> variables = {};

    [[nodiscard]] Value AddVariable(std::shared_ptr<ast::VariableAstNode> node) {
        auto name = node->name;
        variables[name] = node;
        const auto size = node->type.size;
        return Variable{.name = name, .size = size};
    }

    [[nodiscard]] Temp AddTemp(int size) { return Temp{.id = temp_counter++, .size = size}; }

    [[nodiscard]] Label AddLabel() {
        auto label = "L" + std::to_string(label_counter);
        label_counter++;
        return Label{label};
    }
};
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::VariableAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::BinaryOpAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;
auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& ctx)
    -> CondJ;
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::AddrAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;
auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefReadAstNode* node, F_Ctx& ctx)
    -> CondJ;
auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx)
    -> CondJ;
auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx)
    -> CondJ;

[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ReturnAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::MoveAstNode* node,
                                         F_Ctx& ctx) -> CondJ;
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx,
                                         Label true_label, Label false_label) -> CondJ;
[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::JumpAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;

[[nodiscard]] auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::MoveAstNode* node,
                                         F_Ctx& ctx, Label true_label, Label false_label) -> CondJ;
auto munch_stmt(std::vector<Operation>& ops, ast::BodyNode& node, F_Ctx& ctx) -> void;

[[nodiscard]] auto init_new_context() -> F_Ctx;

[[nodiscard]] auto generate_function_prologue_instructions(const ast::FrameAstNode* function,
                                                           F_Ctx& ctx) -> std::vector<Operation>;

[[nodiscard]] auto generate_ir_for_frame(ast::FrameAstNode* function, F_Ctx& ctx) -> Frame;

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx)
    -> void;
auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx) -> void;
auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx) -> void;
auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx) -> void;
auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> void;

[[nodiscard]] auto Produce_IR(std::vector<ast::TopLevelNode>& nodes) -> std::vector<Frame>;

}  // namespace qa_ir