#include "../../include/compiler/assem.hpp"

#include <cassert>
#include <concepts>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

namespace qa_ir {

auto generate_function_prologue_instructions(const ast::FrameAstNode* function, F_Ctx& ctx)
    -> std::vector<Operation> {
    std::vector<Operation> instructions;
    for (const auto [idx, param] : function->params | std::views::enumerate) {
        auto var = std::make_shared<ast::VariableAstNode>(param.name, param.type, std::nullopt); 
        Value dst = ctx.AddVariable(var->name, var->type);
        if (static_cast<size_t>(idx) >= target::param_regs.size()) {
            const auto i = DefineStackPushed{.name = param.name, .size = param.type.size};
            instructions.push_back(i);
        } else {
            const auto param_register = target::param_register_by_convention(idx, param.type.size);
            const auto move_instruction = MovR{.dst = dst, .src = param_register};
            instructions.push_back(move_instruction);
        }
    }

    return instructions;
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::VariableAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };


    if (ctx.variables.find(node->name) == ctx.variables.end()) {
        Value dst = ctx.AddVariable(node->name, node->type); 
        return dst;
    }

    assert(node != nullptr);
    const auto var = ctx.variables.at(node->name);
    Value *offset = nullptr;
    if (node->offset.has_value()) {
        offset = new Value(std::visit(rhs_visitor, node->offset.value().node));

    }

    return Variable{.name = node->name, .type = node->type, .offset = offset};
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    auto lhs_value = std::visit(rhs_visitor, node->lhs.node);
    auto rhs_value = std::visit(rhs_visitor, node->rhs.node);

    static const std::map<ast::BinOpKind, std::function<Operation(Value, Value, Value)>> bin_op_map{
        {ast::BinOpKind::Add,
         [](Value dst, Value left, Value right) -> Operation {
             return Add{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Sub,
         [](Value dst, Value left, Value right) -> Operation {
             return Sub{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Eq,
         [](Value dst, Value left, Value right) -> Operation {
             return Equal{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Gt,
         [](Value dst, Value left, Value right) -> Operation {
             return GreaterThan{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Neq,
         [](Value dst, Value left, Value right) -> Operation {
             return NotEqual{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Lt,
         [](Value dst, Value left, Value right) -> Operation {
             return LessThan{.dst = dst, .left = left, .right = right};
         }},
    };

    auto bin_op = node->kind;
    if (bin_op_map.find(bin_op) == bin_op_map.end()) {
        throw std::runtime_error("Unsupported binary operation " + ast::bin_op_to_string(bin_op));
    }

    auto bin_op_func = bin_op_map.at(bin_op);
    // TODO: fix the size on this
    auto dst = ctx.AddTemp(4);
    auto bin_op_instruction = bin_op_func(dst, lhs_value, rhs_value);
    ops.push_back(bin_op_instruction);
    return dst;
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& ctx) -> Value {
    return ConstInt{.numerical_value = node->value};
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::AddrAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    auto variable = std::visit(rhs_visitor, node->expr.node);
    const auto dst = ctx.AddTemp(target::address_size);
    const auto addr_instruction = Addr{.dst = dst, .src = variable};
    ops.push_back(addr_instruction);
    return dst;
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::DerefReadAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    auto src = std::visit(rhs_visitor, node->base_expr.node);
    auto variable = std::get<Variable>(src);
    const auto astVariable = ctx.variables.at(variable.name);
    const auto variableType = astVariable.type;
    assert(variableType.is_pointer);
    const auto depth = node->deref_depth();

    const auto dest = ctx.AddTemp(variableType.points_to_size);
    const auto deref_instruction = Deref{.dst = dest, .src = src, .depth = depth};
    ops.push_back(deref_instruction);
    return dest;
}



auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx)
    -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };
    auto value_pointing_to = std::visit(rhs_visitor, node->base_expr.node);
    return value_pointing_to;
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error(
        "gen_ir_for_rhs(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx)
    -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    std::vector<Value> args;
    for (auto [idx, arg] : node->callArgs | std::views::enumerate) {
        auto arg_value = std::visit(rhs_visitor, arg.node);
        args.push_back(arg_value);
    }
    const auto return_size = node->returnType.size;
    const auto dst = ctx.AddTemp(return_size);
    const auto call_instruction = Call{.name = node->callName, .args = args, .dst = dst};
    ops.push_back(call_instruction);
    return dst;
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error(
        "gen_ir_for_rhs(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error(
        "gen_ir_for_rhs(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error(
        "gen_ir_for_rhs(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_rhs(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error(
        "gen_ir_for_rhs(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::VariableAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::VariableAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::AddrAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::AddrAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::DerefReadAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::DerefReadAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx)
    -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx) -> void {
    // run the init code like normal
    gen_ir_for_stmt(ops, node->forInit.get(), ctx);

    /** loop conditional then jump label **/
    const auto bottom_loop_label = ctx.AddLabel();
    const auto bottom_loop_label_def_ins = LabelDef{.label = bottom_loop_label};

    // the instruction to jump to the conditional then update label
    auto jump_instruction = Jump{.label = bottom_loop_label};
    ops.emplace_back(jump_instruction);

    auto loop_body_and_update_label = ctx.AddLabel();
    auto loop_body_and_update_label_instruction = LabelDef{.label = loop_body_and_update_label};
    ops.emplace_back(loop_body_and_update_label_instruction);
    for (auto& node : node->forBody) {
        munch_stmt(ops, node, ctx);
    }
    // then the instruction to update the (increment) part of the for loop
    if (node->forUpdate.has_value()) {
        std::visit([&ops, &ctx](auto&& arg) { gen_ir_for_stmt(ops, arg.get(), ctx); },
                   node->forUpdate.value().node);
    }
    // then define the bottom loop label
    ops.emplace_back(bottom_loop_label_def_ins);
    auto exit_label = ctx.AddLabel();
    auto exit_label_instruction = LabelDef{.label = exit_label};

    if (node->forCondition.has_value()) {
        std::visit(
            [&ops, &ctx, &loop_body_and_update_label, &exit_label](auto&& arg) {
                return gen_ir_for_conditonal(ops, arg.get(), ctx, loop_body_and_update_label,
                                             exit_label);
            },
            node->forCondition.value().node);
    } else {
        auto unconditional_jump_back_to_top = Jump{.label = loop_body_and_update_label};
        ops.emplace_back(unconditional_jump_back_to_top);
    }
    ops.emplace_back(exit_label_instruction);
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx)
    -> void {
    gen_ir_for_rhs(ops, node, ctx);
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::VariableAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::VariableAstNode* node, F_Ctx& "
        "ctx)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    auto lhs_value = std::visit(rhs_visitor, node->lhs.node);
    auto rhs_value = std::visit(rhs_visitor, node->rhs.node);
    auto compareInstruction = Compare{.left = lhs_value, .right = rhs_value};
    ops.push_back(compareInstruction);
    std::map<ast::BinOpKind, std::function<CondJ(Value, Value)>> bin_op_map{
        {ast::BinOpKind::Eq,
         [&](Value left, Value right) -> CondJ {
             auto condj = ConditionalJumpEqual{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
             return condj;
         }},
        {ast::BinOpKind::Gt,
         [&](Value left, Value right) -> CondJ {
             auto condj =
                 ConditionalJumpGreater{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
             return condj;
         }},
        {ast::BinOpKind::Neq,
         [&](Value left, Value right) -> CondJ {
             auto condj =
                 ConditionalJumpNotEqual{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
             return condj;
         }},
        {ast::BinOpKind::Lt,
         [&](Value left, Value right) -> CondJ {
             auto condj = ConditionalJumpLess{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
             return condj;
         }},
    };

    auto bin_op = node->kind;
    if (bin_op_map.find(bin_op) == bin_op_map.end()) {
        throw std::runtime_error("Unsupported binary operation.");
    }

    std::function<qa_ir::CondJ(qa_ir::Value, qa_ir::Value)> bin_op_func = bin_op_map.at(bin_op);
    return bin_op_func(lhs_value, rhs_value);
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ConstIntAstNode* node, F_Ctx& "
        "ctx");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::AddrAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::AddrAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefReadAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefReadAstNode* node, "
        "F_Ctx&, Label true_label, Label false_label "
        "ctx)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::DerefWriteAstNode* node, "
        "F_Ctx&, Label true_label, Label false_label "
        "ctx)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ForLoopAstNode* node, F_Ctx& ctx, "
        "Label true_label, Label false_label)");
}
auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx, "
        "Label true_label, Label false_label)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx, "
        "Label true_label, Label false_label)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx, Label "
        "true_label, Label false_label)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx, Label "
        "true_label, Label false_label)");
}

auto gen_ir_for_conditonal(std::vector<Operation>& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx,
                           Label true_label, Label false_label) -> CondJ {
    throw std::runtime_error(
        "gen_ir_for_conditonal(std::vector<Operation>& ops, ast::FunctionCallAstNode* node, "
        "F_Ctx&, Label true_label, Label false_label "
        "ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error(
        "gen_ir_for_stmt(std::vector<Operation>& ops, ast::JumpAstNode* node, F_Ctx& ctx)");
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::IfNode* node, F_Ctx& ctx) -> void {
    auto true_label = ctx.AddLabel();
    auto false_label = ctx.AddLabel();

    const auto conditional_visitor = [&ops, &ctx, &true_label, &false_label](auto&& arg) -> CondJ {
        return gen_ir_for_conditonal(ops, arg.get(), ctx, true_label, false_label);
    };

    auto conditionalJump = std::visit(conditional_visitor, node->condition.node);

    std::vector<Operation> true_instructions = {};

    for (auto& node : node->then) {
        munch_stmt(true_instructions, node, ctx);
    }
    std::vector<Operation> false_instructions = {};
    if (node->else_.has_value()) {
        for (auto& node : *node->else_) {
            munch_stmt(false_instructions, node, ctx);
        }
    }

    // define true branch
    auto then = LabelDef{.label = true_label};
    ops.emplace_back(then);
    std::ranges::copy(true_instructions, std::back_inserter(ops));

    // define false branch
    auto else_ = LabelDef{.label = false_label};
    ops.emplace_back(else_);
    if (node->else_.has_value()) {
        std::ranges::copy(false_instructions, std::back_inserter(ops));
    }
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::ReturnAstNode* node, F_Ctx& ctx) -> void {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_ir_for_rhs(ops, arg.get(), ctx);
    };

    auto return_value = std::visit(rhs_visitor, node->expr.node);
    const auto return_instruction = Ret{.value = return_value};
    ops.push_back(return_instruction);
}

auto gen_ir_for_stmt(std::vector<Operation>& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> void {
    if (!node->rhs.has_value()) {
        auto var = node->lhs.get_variable_name();
        auto var_type = node->lhs.get_variable_type();
        auto dst = ctx.AddVariable(var, var_type);
        return;
    }
  

    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        auto ptr = arg.get();
        assert(ptr != nullptr);
        return gen_ir_for_rhs(ops, ptr, ctx);
    };

    auto src = std::visit(rhs_visitor, node->rhs.value().node);
    if (node->lhs.is_variable_ast_node()) {
        const auto lhs_var = node->lhs.get_variable_ast_node();
        qa_ir::Value dst = ctx.AddVariable(lhs_var->name, lhs_var->type ); 

        if (lhs_var->offset.has_value()) {
            auto offset = std::visit(rhs_visitor, lhs_var->offset.value().node);
            auto offset_v = new Value(offset);
            dst = Variable{.name = lhs_var->name, .type = lhs_var->type, .offset = offset_v};
        }



        const auto move_instruction = Mov{.dst = dst, .src = src};
        ops.push_back(move_instruction);
    } else if (node->lhs.is_deref_write()) {
        auto dst = std::visit(rhs_visitor, node->lhs.node);
        const auto deref_instruction = DerefStore{.dst = dst, .src = src};
        ops.push_back(deref_instruction);
    } else {
        throw std::runtime_error("Unsupported node type.");
    }
}

auto munch_stmt(std::vector<Operation>& ops, ast::BodyNode& node, F_Ctx& ctx) -> void {
    if (node.is_stmt()) {
        auto stmt = node.get_stmt();
        std::visit([&ops, &ctx](auto&& arg) { return gen_ir_for_stmt(ops, arg.get(), ctx); },
                   stmt.node);
        return;
    } else if (node.is_move()) {
        auto move = node.get_move();
        gen_ir_for_stmt(ops, move.get(), ctx);
        return;
    } else {
        throw std::runtime_error("Unsupported node type.");
    }
}

auto generate_ir_for_frame(ast::FrameAstNode* function, F_Ctx& ctx) -> Frame {
    const auto name = function->name;

    std::vector<Operation> func_instructions =
        generate_function_prologue_instructions(function, ctx);
    for (auto& node : function->body) {
        munch_stmt(func_instructions, node, ctx);
    }
    return Frame{.name = name, .instructions = func_instructions};
}

auto init_new_context() -> F_Ctx {
    return F_Ctx{.temp_counter = 0, .label_counter = 0, .variables = {}};
}

auto Produce_IR(std::vector<ast::TopLevelNode>& nodes) -> std::vector<Frame> {
    std::vector<Frame> frames;

    for (auto& node : nodes) {
        auto ctx = init_new_context();
        if (!node.is_function()) {
            throw std::runtime_error("Only support functions at the top level.");
        }

        ast::FrameAstNode* function = node.get_function();
        const auto frame = generate_ir_for_frame(function, ctx);
        frames.push_back(frame);
    }

    return frames;
}
}  // namespace qa_ir
