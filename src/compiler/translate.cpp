#include <utility>

#include "../../include/ast/asttraits.hpp"
#include "../../include/compiler/translate.hpp"

namespace ast {

// primary
auto translate(const std::shared_ptr<st::PrimaryExpression>& expr, Ctx& ctx) -> Stmt {
    if (expr->type == st::PrimaryExpressionType::INT) {
        return std::make_shared<ConstIntAstNode>(expr->value);
    }
    else if (expr->type == st::PrimaryExpressionType::FLOAT) {
        return std::make_shared<ConstFloatNode>(expr->f_value);
    }

    if (expr->type == st::PrimaryExpressionType::IDEN) {
        const auto iden = expr->idenValue;
        if (ctx.local_variables.find(iden) != ctx.local_variables.end()) {
            const auto var = ctx.local_variables[iden];
            return std::make_shared<VariableAstNode>(iden, var->type, std::nullopt);
        }
        throw std::runtime_error("Variable not found: " + iden);
    }
    throw std::runtime_error("translate(st::PrimaryExpression *expr, Ctx &ctx) not implemented");
}

// primary
auto translate(const std::shared_ptr<st::ArrayAccessExpression>& expr, Ctx& ctx) -> Stmt {
    std::string name = expr->name;
    auto index = translate(expr->index, ctx);
    const DataType dt = ctx.local_variables[name]->type;
    return std::make_shared<VariableAstNode>(name, dt, index); 
}


// assignment
auto translate(const std::shared_ptr<st::AssignmentExpression>& expr, Ctx& ctx) -> Stmt {
    ctx.set_lvalueContext("translate(const st::AssignmentExpression &expr, Ctx &ctx)", true);
    auto lhs = translate(expr->lhs, ctx);
    ctx.set_lvalueContext("translate(const st::AssignmentExpression &expr, Ctx &ctx)", false);
    auto rhs = translate(expr->rhs, ctx);

    return std::make_shared<MoveAstNode>(std::move(lhs), std::move(rhs));
}

// unary expression
// assignment
auto translate(const std::shared_ptr<st::UnaryExpression>& expr, Ctx& ctx) -> Stmt {
    auto e = translate(expr->expr, ctx);
    if (expr->type == st::UnaryExpressionType::DEREF && ctx.__lvalueContext == false) {
        return std::make_shared<DerefReadAstNode>(std::move(e), std::nullopt);
    } else if (expr->type == st::UnaryExpressionType::DEREF && ctx.__lvalueContext == true) {
        return std::make_shared<DerefWriteAstNode>(std::move(e), std::nullopt);
    } else if (expr->type == st::UnaryExpressionType::ADDR) {
        return std::make_shared<AddrAstNode>(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::NEG) {
        if (std::holds_alternative<std::shared_ptr<st::PrimaryExpression>>(expr->expr)) {
            auto primary = std::get<std::shared_ptr<st::PrimaryExpression>>(expr->expr);
            if (primary->type == st::PrimaryExpressionType::INT) {
                return std::make_shared<ConstIntAstNode>(-primary->value);
            }
        }
        return std::make_shared<BinaryOpAstNode>(std::make_shared<ConstIntAstNode>(0), std::move(e),
                                                 BinOpKind::Sub);
    }

    throw std::runtime_error("translate(const st::UnaryExpression &expr, Ctx &ctx)");
}

auto translate(const std::shared_ptr<st::AdditiveExpression>& expr, Ctx& ctx) -> Stmt {
    auto lhs = translate(expr->lhs, ctx);
    auto rhs = translate(expr->rhs, ctx);
    std::unordered_map<st::AdditiveExpressionType, BinOpKind> mp = {
        {st::AdditiveExpressionType::ADD, BinOpKind::Add},
        {st::AdditiveExpressionType::SUB, BinOpKind::Sub},
        {st::AdditiveExpressionType::EQ, BinOpKind::Eq},
        {st::AdditiveExpressionType::NEQ, BinOpKind::Neq},
        {st::AdditiveExpressionType::GT, BinOpKind::Gt},
        {st::AdditiveExpressionType::LT, BinOpKind::Lt},
    };
    if (mp.find(expr->type) != mp.end()) {
        return std::make_shared<BinaryOpAstNode>(std::move(lhs), std::move(rhs), mp[expr->type]);
    }
    throw std::runtime_error("translate(const st::AdditiveExpression &expr, Ctx &ctx)");
}

auto translate(const std::shared_ptr<st::ForStatement>& stmt, Ctx& ctx) -> Stmt {
    const st::ForDeclaration& init = stmt->init;
    const auto iden = init.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = ast::toDataType(init);
    ctx.local_variables[iden] = std::make_shared<VariableAstNode>(iden, datatype, std::nullopt);
    const auto& expr = init.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext("translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
                          false);
    auto initInFirstEntryOfForLoop = translate(expr, ctx);
    ctx.set_lvalueContext("translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
                          true);

    auto var = std::make_shared<VariableAstNode>(iden, datatype, std::nullopt);
    auto forInit =
        std::make_shared<MoveAstNode>(std::move(var), std::move(initInFirstEntryOfForLoop));

    std::optional<Stmt> forCondition;
    std::optional<Stmt> forUpdate;
    if (stmt->cond) {
        forCondition = translate(*stmt->cond, ctx);
    }
    if (stmt->inc) {
        forUpdate = translate(*stmt->inc, ctx);
    }
    if (stmt->body == nullptr) {
        throw std::runtime_error("for body is null");
    }
    auto body = translate(*stmt->body.get(), ctx);

    return std::make_shared<ForLoopAstNode>(std::move(forInit), std::move(forCondition),
                                            std::move(forUpdate), std::move(body));
}

auto translate(const std::shared_ptr<st::FunctionCallExpression>& expr, Ctx& ctx) -> Stmt {
    std::vector<Stmt> args;
    for (const auto& arg : expr->args) {
        auto e = translate(arg, ctx);
        args.push_back(std::move(e));
    }

    const auto faux_return_type =
        DataType{.name = "int", .size = 4, .is_pointer = false, .points_to_size = 0};

    return std::make_shared<FunctionCallAstNode>(expr->name, std::move(args), faux_return_type);
}

// expression
auto translate(const st::Expression& expr, Ctx& ctx) -> Stmt {
    // TODO: fix. can lead to recursive call
    return std::visit([&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, expr);
}

// return statement
auto translate(const std::shared_ptr<st::ReturnStatement>& stmt, Ctx& ctx) -> Stmt {
    ctx.set_lvalueContext("translate(const st::ReturnStatement &stmt, Ctx &ctx)", false);
    auto expr = translate(stmt->expr, ctx);
    ctx.set_lvalueContext("translate(const st::ReturnStatement &stmt, Ctx &ctx)", true);
    return std::make_shared<ReturnAstNode>(std::move(expr));
}

// expression statement
auto translate(const std::shared_ptr<st::ExpressionStatement>& stmt, Ctx& ctx) -> Stmt {
    return translate(stmt->expr, ctx);
}

// selection statement statement
auto translate(const std::shared_ptr<st::SelectionStatement>& stmt, Ctx& ctx) -> Stmt {
    auto condition = translate(stmt->cond, ctx);
    if (const auto kind = condition.get_bin_op()) {
        if (!is_comparison(*kind)) {
            condition = std::make_shared<BinaryOpAstNode>(
                std::move(condition), std::make_shared<ConstIntAstNode>(0), BinOpKind::Neq);
        }
        auto then = translate(*stmt->then, ctx);
        if (stmt->else_) {
            auto else_ = translate(*stmt->else_, ctx);
            return std::make_shared<IfNode>(std::move(condition), std::move(then),
                                            std::move(else_));
        }
        return std::make_shared<IfNode>(std::move(condition), std::move(then), std::nullopt);
    } else {
        condition = std::make_shared<BinaryOpAstNode>(
            std::move(condition), std::make_shared<ConstIntAstNode>(0), BinOpKind::Neq);
    }
    return std::make_shared<IfNode>(
        std::move(condition), translate(*stmt->then, ctx),
        stmt->else_ ? std::make_optional(translate(*stmt->else_, ctx)) : std::nullopt);
}

// declaration
[[nodiscard]] auto translate(const st::Declaration& decl, Ctx& ctx)
    -> std::shared_ptr<MoveAstNode> {
    const auto iden = decl.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = ast::toDataType(decl);
    const auto var = std::make_shared<VariableAstNode>(iden, datatype, std::nullopt);
    ctx.local_variables[iden] = var;

    if (!decl.initDeclarator.value().initializer.has_value()) {
        return std::make_shared<MoveAstNode>(
            std::make_shared<VariableAstNode>(iden, datatype, std::nullopt),
            std::nullopt); 
    }

    const auto& expr = decl.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)", false);
    auto init = translate(expr, ctx);
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)", true);

    return std::make_shared<MoveAstNode>(std::move(var), std::move(init));
}

[[nodiscard]] std::vector<FrameParam> translate(st::ParamTypeList& params) {
    std::vector<FrameParam> result;
    for (const auto& p : params.params) {
        const auto name = p.Name();
        const auto type = ast::toDataType(p);
        const auto fp = FrameParam{
            .name = name,
            .type = type,
        };
        result.push_back(fp);
    }
    return result;
}

auto translateStatement(st::Statement& stmt, Ctx& ctx) -> Stmt {
    auto s = std::move(stmt.stmt);
    return std::visit([&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, s);
}

auto translate(st::CompoundStatement& stmts, Ctx& ctx) -> std::vector<BodyNode> {
    if (stmts.items.empty()) {
        return {};
    }
    std::vector<BodyNode> result;
    for (auto& bi : stmts.items) {
        if (std::holds_alternative<st::Statement>(bi.item)) {
            auto stmt = std::get<st::Statement>(std::move(bi.item));
            auto node = translateStatement(stmt, ctx);
            result.push_back(std::move(node));
        } else {
            auto& decl = std::get<st::Declaration>(bi.item);
            auto node = translate(decl, ctx);
            result.push_back(std::move(node));
        }
    }
    return result;
}

auto translate(st::FuncDef* fd, Ctx& ctx) -> std::shared_ptr<FrameAstNode> {
    const auto functionName = fd->Name();
    auto functionParams = fd->DirectDeclarator().params;
    auto params = translate(functionParams);
    for (const auto& p : params) {
        const auto paramName = p.name;
        const auto type = p.type;
        ctx.local_variables[p.name] = std::make_shared<VariableAstNode>(paramName, type, std::nullopt) ; 
    }
    auto body = translate(fd->body, ctx);
    return std::make_shared<FrameAstNode>(functionName, std::move(body), std::move(params));
}

auto translate(const st::ExternalDeclaration& node, Ctx& ctx) -> TopLevelNode {
    const auto& nv = node.node;
    if (std::holds_alternative<std::shared_ptr<st::FuncDef>>(nv)) {
        auto funcdef = std::get<std::shared_ptr<st::FuncDef>>(nv).get();
        return TopLevelNode{translate(funcdef, ctx)};
    }
    const auto& decl = std::get<st::Declaration>(nv);

    return TopLevelNode{translate(decl, ctx)};
}

[[nodiscard]] std::vector<TopLevelNode> translate(const st::Program& program) {
    auto ctx = Ctx{
        .counter = 0,
        .local_variables = {},
    };
    std::vector<TopLevelNode> nodes;
    for (const auto& decl : program.nodes) {
        auto node = translate(decl, ctx);
        nodes.push_back(std::move(node));
    }
    return nodes;
}
}  // namespace ast
