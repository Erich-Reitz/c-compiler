#include "../../../include/compiler/qa_ir/assem.hpp"

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

namespace {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

using op_list = std::vector<Operation>;

auto gen_cond(op_list& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx, Label true_label,
              Label false_label) -> void;

auto gen_stmt(op_list& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx) -> void;
auto gen_stmt(op_list& ops, ast::JumpAstNode* node, F_Ctx& ctx) -> void;
auto gen_stmt(op_list& ops, ast::IfNode* node, F_Ctx& ctx) -> void;
auto gen_stmt(op_list& ops, ast::ReturnAstNode* node, F_Ctx& ctx) -> void;
auto gen_stmt(op_list& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> void;

[[nodiscard]] auto gen_rhs(op_list& ops, ast::VariableAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx) -> Value;

[[nodiscard]] auto gen_rhs(op_list& ops, ast::DerefReadAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::AddrAstNode* node, F_Ctx& ctx) -> Value;

[[nodiscard]] auto gen_rhs(op_list& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::ConstIntAstNode* node, F_Ctx& ctx) -> Value;
[[nodiscard]] auto gen_rhs(op_list& ops, ast::ConstFloatNode* node, F_Ctx& ctx) -> Value;

auto munch_stmt(op_list& ops, ast::BodyNode& node, F_Ctx& ctx) -> void;

[[nodiscard]] auto gen_fun_prologue(const ast::FrameAstNode* function, F_Ctx& ctx) -> op_list;

auto gen_fun_prologue(const ast::FrameAstNode* function, F_Ctx& ctx) -> op_list {
    op_list instructions;
    for (const auto [idx, param] : function->params | std::views::enumerate) {
        auto var = std::make_shared<ast::VariableAstNode>(param.name, param.type);
        Value dst = ctx.AddVariable(var->name, var->type);
        if (static_cast<size_t>(idx) >= target::param_regs.size()) {
            const auto i = DefineStackPushed{.name = param.name, .size = param.type.GetSize()};
            instructions.push_back(i);
        } else {
            const auto param_register =
                target::param_register_by_convention(idx, param.type.GetSize());
            const auto move_instruction = MovR{.dst = dst, .src = param_register};
            instructions.push_back(move_instruction);
        }
    }

    return instructions;
}

auto gen_rhs(op_list& ops, ast::VariableAstNode* node, F_Ctx& ctx) -> Value {
    if (ctx.variables.find(node->name) == ctx.variables.end()) {
        Value dst = ctx.AddVariable(node->name, node->type);
        return dst;
    }

    assert(node != nullptr);
    const auto var = ctx.variables.at(node->name);
    const auto result = Variable{.name = node->name, .type = node->type};
    return result;
}

auto gen_rhs(op_list& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    auto lhs_value = std::visit(rhs_visitor, node->lhs.node);
    auto rhs_value = std::visit(rhs_visitor, node->rhs.node);

    static const std::map<ast::BinOpKind, std::function<Operation(Value, Value, Value)>> bin_op_map{
        {ast::BinOpKind::Add,
         [](Value dst, Value left, Value right) -> Operation {
             if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
                 return Add<ast::BaseType::INT, ast::BaseType::INT>{
                     .dst = dst, .left = left, .right = right};
             }
             if (GetDataType(left).is_float() && GetDataType(right).is_float()) {
                 return Add<ast::BaseType::FLOAT, ast::BaseType::FLOAT>{
                     .dst = dst, .left = left, .right = right};
             }
             throw std::runtime_error("invalid types for add");
         }},
        {ast::BinOpKind::Sub,
         [](Value dst, Value left, Value right) -> Operation {
             return Sub{.dst = dst, .left = left, .right = right};
         }},
        {ast::BinOpKind::Eq,
         [](Value dst, Value left, Value right) -> Operation {
             if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
                 return Equal<ast::BaseType::INT, ast::BaseType::INT>{
                     .dst = dst, .left = left, .right = right};
             }
             if (GetDataType(left).is_float() && GetDataType(right).is_float()) {
                 return Equal<ast::BaseType::FLOAT, ast::BaseType::FLOAT>{
                     .dst = dst, .left = left, .right = right};
             }
             throw std::runtime_error("invalid types for not equal");
         }},
        {ast::BinOpKind::Gt,
         [](Value dst, Value left, Value right) -> Operation {
             if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
                 return GreaterThan<ast::BaseType::INT, ast::BaseType::INT>{
                     .dst = dst, .left = left, .right = right};
             }
             if (GetDataType(left).is_float() && GetDataType(right).is_float()) {
                 return GreaterThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>{
                     .dst = dst, .left = left, .right = right};
             }
             throw std::runtime_error("invalid types for less than");
         }},
        {ast::BinOpKind::Neq,
         [](Value dst, Value left, Value right) -> Operation {
             if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
                 return NotEqual<ast::BaseType::INT, ast::BaseType::INT>{
                     .dst = dst, .left = left, .right = right};
             }
             if (GetDataType(left).is_float() && GetDataType(right).is_float()) {
                 return NotEqual<ast::BaseType::FLOAT, ast::BaseType::FLOAT>{
                     .dst = dst, .left = left, .right = right};
             }
             throw std::runtime_error("invalid types for not equal");
         }},
        {ast::BinOpKind::Lt,
         [](Value dst, Value left, Value right) -> Operation {
             if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
                 return LessThan<ast::BaseType::INT, ast::BaseType::INT>{
                     .dst = dst, .left = left, .right = right};
             }
             if (GetDataType(left).is_float() && GetDataType(right).is_float()) {
                 return LessThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>{
                     .dst = dst, .left = left, .right = right};
             }
             throw std::runtime_error("invalid types for less than");
         }},
    };

    static const std::map<ast::BinOpKind, std::function<Operation(Value, Value, Value)>>
        pointer_arth_map{
            {ast::BinOpKind::Add,
             [](Value dst, Value left, Value right) -> Operation {
                 if (GetDataType(left).base_type == ast::BaseType::POINTER ||
                     GetDataType(left).base_type == ast::BaseType::ARRAY) {
                     assert(GetDataType(right).base_type != ast::BaseType::POINTER);
                     return PointerOffset{.dst = dst,
                                          .basisType = GetDataType(right),
                                          .base = left,
                                          .offset = right};
                 } else {
                     assert(GetDataType(left).base_type != ast::BaseType::POINTER);
                     return PointerOffset{
                         .dst = dst, .basisType = GetDataType(left), .base = right, .offset = left};
                 }
             }},
        };

    auto bin_op = node->kind;
    if (bin_op_map.find(bin_op) == bin_op_map.end()) {
        throw std::runtime_error("Unsupported binary operation " + ast::bin_op_to_string(bin_op));
    }

    auto bin_op_func = bin_op_map.at(bin_op);
    // TODO: fix the size on this
    const auto lhs_type = GetDataType(lhs_value);
    const auto rhs_type = GetDataType(rhs_value);

    const auto resulting_type = ResultingTypeForBinOp(lhs_type, rhs_type, bin_op);
    auto dst = ctx.AddTemp(resulting_type);
    if (resulting_type.base_type == ast::BaseType::POINTER) {
        if (pointer_arth_map.find(bin_op) == pointer_arth_map.end()) {
            throw std::runtime_error("Unsupported binary operation " +
                                     ast::bin_op_to_string(bin_op));
        }

        bin_op_func = pointer_arth_map.at(bin_op);
        ops.push_back(bin_op_func(dst, lhs_value, rhs_value));
        return dst;
    }

    auto bin_op_instruction = bin_op_func(dst, lhs_value, rhs_value);
    ops.push_back(bin_op_instruction);
    return dst;
}

[[nodiscard]] auto gen_rhs(op_list& ops, ast::ConstFloatNode* node, F_Ctx& ctx) -> Value {
    return Immediate<float>{.numerical_value = node->value};
}

auto gen_rhs(op_list& ops, ast::ConstIntAstNode* node, F_Ctx& ctx) -> Value {
    return Immediate<int>{.numerical_value = node->value};
}

auto gen_rhs(op_list& ops, ast::AddrAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    auto variable = std::visit(rhs_visitor, node->expr.node);
    const auto variable_type = GetDataType(variable);
    const auto previous_level = variable_type.indirect_level;
    const auto dst = ctx.AddTemp(ast::DataType{.base_type = ast::BaseType::POINTER,
                                               .points_to = variable_type.base_type,
                                               .array_size = 0,
                                               .indirect_level = previous_level + 1});
    const auto addr_instruction = Addr{.dst = dst, .src = variable};
    ops.push_back(addr_instruction);
    return dst;
}

auto gen_rhs(op_list& ops, ast::DerefReadAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    auto src = std::visit(rhs_visitor, node->expr.node);
    if (std::holds_alternative<Variable>(src)) {
        const auto variable = std::get<Variable>(src);
        assert(variable.type.base_type == ast::BaseType::POINTER ||
               variable.type.base_type == ast::BaseType::ARRAY);
        const auto depth = node->deref_depth();

        auto dest = ctx.AddTemp(ast::dereference_type(variable.type));
        const auto deref_instruction = Deref{.dst = dest, .src = src, .depth = depth};
        ops.push_back(deref_instruction);
        return dest;
    }

    if (std::holds_alternative<Temp>(src)) {
        const auto variable = std::get<Temp>(src);
        const auto variableType = variable.type;
        assert(variableType.base_type == ast::BaseType::POINTER ||
               variableType.base_type == ast::BaseType::ARRAY);
        const auto depth = node->deref_depth();
        // TODO: arbitrary pointers** is messing this up
        const auto indirect_level = variableType.indirect_level;
        auto dest = ctx.AddTemp(ast::DataType{.base_type = variableType.points_to,
                                              .points_to = ast::BaseType::NONE,
                                              .array_size = 0,
                                              .indirect_level = indirect_level - 1});
        if (dest.type.indirect_level != 0) {
            dest.type.points_to = variableType.points_to;
        }
        const auto deref_instruction = Deref{.dst = dest, .src = src, .depth = depth};
        ops.push_back(deref_instruction);
        return dest;
    }
    throw std::runtime_error("Unsupported node type.");
}

auto gen_rhs(op_list& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };
    auto value_pointing_to = std::visit(rhs_visitor, node->expr.node);
    return value_pointing_to;
}

auto gen_rhs(op_list& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx) -> Value {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    std::vector<Value> args;
    for (auto [idx, arg] : node->callArgs | std::views::enumerate) {
        auto arg_value = std::visit(rhs_visitor, arg.node);
        args.push_back(arg_value);
    }

    const auto dst = ctx.AddTemp(node->returnType);
    const auto call_instruction = Call{.name = node->callName, .args = args, .dst = dst};
    ops.push_back(call_instruction);
    return dst;
}

auto gen_rhs(op_list& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> Value {
    throw std::runtime_error("gen_rhs(op_list &ops, ast::MoveAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::VariableAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::VariableAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::BinaryOpAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::ConstFloatNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::ConstIntAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::ConstIntAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::ConstIntAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::AddrAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::AddrAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::DerefReadAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::DerefReadAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::DerefWriteAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::DerefWriteAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::ForLoopAstNode* node, F_Ctx& ctx) -> void {
    // run the init code like normal
    gen_stmt(ops, node->forInit.get(), ctx);

    /** loop conditional then jump label **/
    const auto bottom_loop_label = ctx.AddLabel();
    const auto bottom_loop_label_def_ins = LabelDef{.label = bottom_loop_label};

    // the instruction to jump to the conditional then update label
    auto jump_instruction = Jump{.label = bottom_loop_label};
    ops.emplace_back(jump_instruction);

    auto loop_body_and_update_label = ctx.AddLabel();
    auto loop_body_and_update_label_instruction = LabelDef{.label = loop_body_and_update_label};
    ops.emplace_back(loop_body_and_update_label_instruction);
    for (auto& body_node : node->forBody) {
        munch_stmt(ops, body_node, ctx);
    }
    // then the instruction to update the (increment) part of the for loop
    if (node->forUpdate.has_value()) {
        std::visit([&ops, &ctx](auto&& arg) { gen_stmt(ops, arg.get(), ctx); },
                   node->forUpdate.value().node);
    }
    // then define the bottom loop label
    ops.emplace_back(bottom_loop_label_def_ins);
    auto exit_label = ctx.AddLabel();
    auto exit_label_instruction = LabelDef{.label = exit_label};

    if (node->forCondition.has_value()) {
        gen_cond(ops, node->forCondition.value().get(), ctx, loop_body_and_update_label,
                 exit_label);
    } else {
        auto unconditional_jump_back_to_top = Jump{.label = loop_body_and_update_label};
        ops.emplace_back(unconditional_jump_back_to_top);
    }
    ops.emplace_back(exit_label_instruction);
}

auto gen_stmt(op_list& ops, ast::FunctionCallAstNode* node, F_Ctx& ctx) -> void {
    auto result = gen_rhs(ops, node, ctx);
}

auto generate_compare(Value left, Value right) -> decltype(auto) {
    if (GetDataType(left).is_int() && GetDataType(right).is_int()) {
        return Compare<ast::BaseType::INT, ast::BaseType::INT>{.left = left, .right = right};
    }

    throw std::runtime_error("invalid types for compare");
}

auto gen_cond(op_list& ops, ast::BinaryOpAstNode* node, F_Ctx& ctx, Label true_label,
              Label false_label) -> void {
    auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    auto lhs_value = std::visit(rhs_visitor, node->lhs.node);
    auto rhs_value = std::visit(rhs_visitor, node->rhs.node);

    auto compareInstruction = generate_compare(lhs_value, rhs_value);
    ops.push_back(compareInstruction);
    std::map<ast::BinOpKind, std::function<void(Value, Value)>> bin_op_map{
        {ast::BinOpKind::Eq,
         [&](Value left, Value right) -> void {
             auto condj = ConditionalJumpEqual{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
         }},
        {ast::BinOpKind::Gt,
         [&](Value left, Value right) -> void {
             auto condj =
                 ConditionalJumpGreater{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
         }},
        {ast::BinOpKind::Neq,
         [&](Value left, Value right) -> void {
             auto condj =
                 ConditionalJumpNotEqual{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
         }},
        {ast::BinOpKind::Lt,
         [&](Value left, Value right) -> void {
             auto condj = ConditionalJumpLess{.trueLabel = true_label, .falseLabel = false_label};
             ops.push_back(condj);
         }},
    };

    auto bin_op = node->kind;
    if (bin_op_map.find(bin_op) == bin_op_map.end()) {
        throw std::runtime_error("Unsupported binary operation.");
    }

    std::function<void(qa_ir::Value, qa_ir::Value)> bin_op_func = bin_op_map.at(bin_op);

    bin_op_func(lhs_value, rhs_value);
}

auto gen_stmt(op_list& ops, ast::JumpAstNode* node, F_Ctx& ctx) -> void {
    throw std::runtime_error("gen_stmt(op_list &ops, ast::JumpAstNode* node, F_Ctx& ctx)");
}

auto gen_stmt(op_list& ops, ast::IfNode* node, F_Ctx& ctx) -> void {
    auto true_label = ctx.AddLabel();
    auto false_label = ctx.AddLabel();

    gen_cond(ops, node->condition.get(), ctx, true_label, false_label);

    op_list true_instructions = {};

    for (auto& then_node : node->then) {
        munch_stmt(true_instructions, then_node, ctx);
    }
    op_list false_instructions = {};
    if (node->else_.has_value()) {
        for (auto& else_node : *node->else_) {
            munch_stmt(false_instructions, else_node, ctx);
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

auto gen_stmt(op_list& ops, ast::ReturnAstNode* node, F_Ctx& ctx) -> void {
    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        return gen_rhs(ops, arg.get(), ctx);
    };

    auto return_value = std::visit(rhs_visitor, node->expr.node);
    const auto return_instruction = Ret{.value = return_value};
    ops.push_back(return_instruction);
}

auto gen_stmt(op_list& ops, ast::MoveAstNode* node, F_Ctx& ctx) -> void {
    if (!node->rhs.has_value()) {
        auto var = node->lhs.get_variable_name();
        auto var_type = node->lhs.get_variable_type();
        auto dst = ctx.AddVariable(var, var_type);
        // TODO: this is a hack for getting arrays to be defined in the next pass
        if (var_type.base_type == ast::BaseType::ARRAY) {
            ops.push_back(DefineArray{.name = var, .type = var_type});
        }
        return;
    }

    const auto rhs_visitor = [&ops, &ctx](auto&& arg) -> qa_ir::Value {
        auto ptr = arg.get();
        assert(ptr != nullptr);
        return gen_rhs(ops, ptr, ctx);
    };

    auto src = std::visit(rhs_visitor, node->rhs.value().node);
    if (node->lhs.is_variable_ast_node()) {
        const auto lhs_var = node->lhs.get_variable_ast_node();
        qa_ir::Value dst = ctx.AddVariable(lhs_var->name, lhs_var->type);
        const auto move_instruction = Mov{.dst = dst, .src = src};
        ops.push_back(move_instruction);
    } else if (node->lhs.is_deref_write()) {
        auto dst = std::visit(rhs_visitor, node->lhs.node);
        const auto dst_datatype = GetDataType(dst);
        if (dst_datatype.base_type == ast::BaseType::POINTER) {
            const auto deref_instruction = DerefStore{.dst = dst, .src = src};
            ops.push_back(deref_instruction);
        } else {
            const auto move_instruction = Mov{.dst = dst, .src = src};
            ops.push_back(move_instruction);
        }
    } else {
        throw std::runtime_error("Unsupported node type.");
    }
}

auto munch_stmt(op_list& ops, ast::BodyNode& node, F_Ctx& ctx) -> void {
    if (node.is_stmt()) {
        auto stmt = node.get_stmt();
        std::visit([&ops, &ctx](auto&& arg) { return gen_stmt(ops, arg.get(), ctx); }, stmt.node);
        return;
    } else if (node.is_move()) {
        auto move = node.get_move();
        gen_stmt(ops, move.get(), ctx);
        return;
    } else {
        throw std::runtime_error("Unsupported node type.");
    }
}

auto generate_ir_for_frame(ast::FrameAstNode* function, F_Ctx& ctx) -> Frame {
    const auto name = function->name;

    op_list func_instructions = gen_fun_prologue(function, ctx);
    for (auto& node : function->body) {
        munch_stmt(func_instructions, node, ctx);
    }
    return Frame{.name = name, .instructions = func_instructions};
}

#pragma GCC diagnostic pop
}  // namespace

auto Produce_IR(std::vector<ast::TopLevelNode>& nodes) -> std::vector<Frame> {
    std::vector<Frame> frames;

    for (auto& node : nodes) {
        auto ctx = F_Ctx{.temp_counter = 0, .label_counter = 0, .variables = {}};
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
