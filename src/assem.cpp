#include <cassert>
#include <map>
#include <stdexcept>

#include "assem.hpp"
#include "location.hpp"
#include "qa_x86.hpp"

namespace qa_ir {

struct Ctx {
    int counter;
    int labelCounter;
    std::map<std::string, int> variableUsage;
    std::map<std::string, const ast::Node *> variables;

    Temp newTemp(int size) {
        assert(size != 0);
        return Temp{counter++, size};
    }

    Value AddVariable(const ast::Node *node) {
        auto name = node->variableName;
        variableUsage[name]++;
        variables[name] = node;
        const auto size = node->variableType.size;
        return Variable{name, variableUsage[name], size};
    }

    Value getVariable(const std::string &name) {
        return Variable{name, variableUsage[name],
                        variables[name]->variableType.size};
    }

    Label newLabel() {
        auto label = "L" + std::to_string(labelCounter);
        labelCounter++;
        return Label{label};
    }
};

void MunchStmt(std::vector<Operation> &ins,
               const std::unique_ptr<ast::Node> &node, Ctx &ctx);

Value GenerateIRForRhs(std::vector<Operation> &ins, const ast::Node *node,
                       Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::ConstInt: {
        return node->value;
    }
    case ast::NodeType::Var: {
        return ctx.getVariable(node->variableName);
    }
    case ast::NodeType::BinOp: {
        auto lhs = GenerateIRForRhs(ins, node->lhs.get(), ctx);
        auto rhs = GenerateIRForRhs(ins, node->rhs.get(), ctx);
        auto dst = ctx.newTemp(SizeOf(lhs));
        if (node->binOpKind == ast::BinOpKind::Add) {
            auto binop_instruction = Add{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        } else if (node->binOpKind == ast::BinOpKind::Sub) {
            auto binop_instruction = Sub{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        } else if (node->binOpKind == ast::BinOpKind::Eq) {
            auto binop_instruction = Equal{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        } else if (node->binOpKind == ast::BinOpKind::Gt) {
            auto binop_instruction = GreaterThan{.dst = dst, .left = lhs, .right = rhs};
            ins.push_back(binop_instruction);
            return dst;
        } else {
            throw std::runtime_error(
                "GenerateIRForRhs not implemented for binop: " +
                std::to_string(static_cast<int>(node->binOpKind)));
        }
        return dst;
    }
    case ast::NodeType::Call: {
        std::vector<Value> args;
        for (auto [arg, idx] = std::tuple{node->callArgs.begin(), 0}; arg != node->callArgs.end(); ++arg, ++idx) {
            args.push_back(GenerateIRForRhs(ins, arg->get(), ctx));
        }
        const auto returnSize = node->returnType.size;
        auto dst = ctx.newTemp(returnSize);
        auto call_instruction =
            Call{.name = node->callName, .args = args, .dst = dst};
        ins.push_back(call_instruction);
        return dst;
    }
    case ast::NodeType::Addr: {
        auto src = GenerateIRForRhs(ins, node->expr.get(), ctx);
        auto dst = ctx.newTemp(8);
        auto addr_instruction = Addr{.dst = dst, .src = src};
        ins.push_back(addr_instruction);
        return dst;
    }
    case ast::NodeType::Deref: {
        auto src = GenerateIRForRhs(ins, node->expr.get(), ctx);
        auto variable = std::get<Variable>(src);
        const auto var = ctx.variables.at(variable.name);
        const auto varDataType = var->variableType;
        assert(varDataType.pointsTo != nullptr);
        const auto depth = node->derefDepth;
        auto dst = ctx.newTemp(varDataType.pointsTo->size);
        auto deref_instruction = Deref{.dst = dst, .src = src, .depth = depth};
        ins.push_back(deref_instruction);
        return dst;
    }
    default:
        throw std::runtime_error("GenerateIRForRhs not implemented for type: " +
                                 std::to_string(static_cast<int>(node->type)));
    }
}
void GenerateIRForReturnNode(std::vector<Operation> &ins,
                             const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    auto ret = GenerateIRForRhs(ins, node->expr.get(), ctx);
    auto ret_instruction = Ret{.value = ret};
    ins.push_back(ret_instruction);
}

void GenerateIRForMovNode(std::vector<Operation> &ins,
                          const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    auto src = GenerateIRForRhs(ins, node->rhs.get(), ctx);
    switch (node->lhs->type) {
    case ast::NodeType::Var: {
        auto dst = ctx.AddVariable(node->lhs.get());
        auto mov_instruction = Mov{.dst = dst, .src = src};
        ins.push_back(mov_instruction);
        return;
    }
    case ast::NodeType::Deref: {
        auto dst = GenerateIRForRhs(ins, node->lhs->expr.get(), ctx);
        auto deref_instruction = DerefStore{.dst = dst, .src = src};
        ins.push_back(deref_instruction);
        return;
    }
    default:
        throw std::runtime_error("GenerateIRForMovNode not implemented for type: " +
                                 std::to_string(static_cast<int>(node->lhs->type)));
    }
}
/*
* Things like .. if (a) have already been converted to if (a == 0) with flipped then / else
*/
CondJ GenerateConditionalIR(std::vector<Operation> &ins,
                            const std::unique_ptr<ast::Node> &condition, Ctx &ctx) {
    std::vector<Operation> instructions;
    auto then_label = ctx.newLabel();
    auto else_label = ctx.newLabel();
    auto lhs = GenerateIRForRhs(instructions, condition->lhs.get(), ctx);
    auto rhs = GenerateIRForRhs(instructions, condition->rhs.get(), ctx);
    auto cmp_instruction = Compare{.left = lhs, .right = rhs};
    instructions.push_back(cmp_instruction);
    if (condition->binOpKind == ast::BinOpKind::Eq) {
        auto conditional_jump_instruction =
            ConditionalJumpEqual{.trueLabel = then_label, .falseLabel = else_label};
        instructions.push_back(conditional_jump_instruction);
        ins.insert(ins.end(), instructions.begin(), instructions.end());
        return conditional_jump_instruction;
    }
    if (condition->binOpKind == ast::BinOpKind::Gt) {
        auto conditional_jump_instruction = ConditionalJumpGreater{.trueLabel = then_label, .falseLabel = else_label};
        instructions.push_back(conditional_jump_instruction);
        ins.insert(ins.end(), instructions.begin(), instructions.end());
        return conditional_jump_instruction;
    }
    throw std::runtime_error(
        "GenerateConditionalIR not implemented for binop: " +
        std::to_string(static_cast<int>(condition->binOpKind)));
}

void GenerateIRForIf(std::vector<Operation> &ins,
                     const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    auto conditionalJump = GenerateConditionalIR(ins, node->condition, ctx);
    auto then_label = get_true_label(conditionalJump);
    auto else_label = get_false_label(conditionalJump);
    auto then_instructions = std::vector<Operation> {};
    for (const auto &stmt : node->then) {
        MunchStmt(then_instructions, stmt, ctx);
    }
    auto else_instructions = std::vector<Operation> {};
    for (const auto &stmt : node->else_) {
        MunchStmt(else_instructions, stmt, ctx);
    }
    auto then_jump_instruction = LabelDef{.label = then_label};
    ins.push_back(then_jump_instruction);
    ins.insert(ins.end(), then_instructions.begin(), then_instructions.end());
    auto else_jump_instruction = LabelDef{.label = else_label};
    ins.push_back(else_jump_instruction);
    if (else_instructions.size() > 0) {
        ins.insert(ins.end(), else_instructions.begin(), else_instructions.end());
    }
}

void MunchStmt(std::vector<Operation> &ins,
               const std::unique_ptr<ast::Node> &node, Ctx &ctx) {
    switch (node->type) {
    case ast::NodeType::Return: {
        GenerateIRForReturnNode(ins, node, ctx);
        return;
    }
    case ast::NodeType::Move: {
        GenerateIRForMovNode(ins, node, ctx);
        return;
    }
    case ast::NodeType::If: {
        GenerateIRForIf(ins, node, ctx);
        return;
    }
    case ast::NodeType::Call: {
        auto dst = GenerateIRForRhs(ins, node.get(), ctx);
        return;
    }
    default:
        throw std::runtime_error("MunchStmt not implemented for type: " +
                                 std::to_string(static_cast<int>(node->type)));
    }
}

[[nodiscard]] std::vector<Frame>
Produce_IR(const std::vector<std::unique_ptr<ast::Node>> &nodes) {
    std::vector<Frame> frames;
    for (const auto &node : nodes) {
        auto ctx = Ctx{.counter = 0, .labelCounter = 0};
        if (node->type != ast::NodeType::Frame) {
            continue;
        }
        const auto name = node->functionName;
        std::vector<Operation> instructions;
        for (auto [p, idx] = std::tuple{node->params.begin(), 0}; p != node->params.end(); ++p, ++idx) {
            if (idx >= target::param_regs.size()) {
                auto var = ast::makeNewVar(p->name, p->type);
                auto dst = ctx.AddVariable(var.get());
                auto i = DefineStackPushed{.name = p->name, .size = p->type.size};
                instructions.push_back(i);
                continue;
            }
            // create a variable node for the paramter
            auto var = ast::makeNewVar(p->name, p->type);
            // create a stack location for the variable
            auto dst = ctx.AddVariable(var.get());
            const auto param_register = target::param_regs.at(idx);
            auto src = HardcodedRegister{.reg = param_register, .size = p->type.size};
            instructions.push_back(MovR{.dst = dst, .src = src});
        }
        auto body = std::move(node->body);
        for (const auto &item : body) {
            MunchStmt(instructions, item, ctx);
        }
        auto frame = Frame{name, instructions};
        frames.push_back(frame);
    }
    return frames;
}
} // namespace qa_ir
