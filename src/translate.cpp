
#include "../include/translate.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>

#include "../include/asttraits.hpp"

// primary
auto translate(const std::unique_ptr<st::PrimaryExpression>& expr, Ctx& ctx)
    -> std::unique_ptr<ast::Node> {
    if (expr->type == st::PrimaryExpressionType::INT) {
        return ast::makeConstInt(expr->value);
    }
    if (expr->type == st::PrimaryExpressionType::IDEN) {
        const auto& iden = expr->idenValue;
        if (ctx.local_variables.find(iden) != ctx.local_variables.end()) {
            const auto& var = ctx.local_variables[iden];
            return ast::makeNewVar(iden, var->variableType);
        }
        throw std::runtime_error("Variable not found: " + iden);
    }
    throw std::runtime_error(
        "translate(st::PrimaryExpression *expr, Ctx &ctx) not implemented");
}

// assignment
[[nodiscard]] std::unique_ptr<ast::Node> translate(
    const std::unique_ptr<st::AssignmentExpression>& expr, Ctx& ctx) {
    ctx.set_lvalueContext(
        "translate(const st::AssignmentExpression &expr, Ctx &ctx)", true);
    auto lhs = translate(expr->lhs, ctx);
    ctx.set_lvalueContext(
        "translate(const st::AssignmentExpression &expr, Ctx &ctx)", false);
    auto rhs = translate(expr->rhs, ctx);
    return ast::makeNewMove(std::move(lhs), std::move(rhs));
}

// unary expression
// assignment
[[nodiscard]] auto translate(const std::unique_ptr<st::UnaryExpression>& expr,
                             Ctx& ctx) -> std::unique_ptr<ast::Node> {
    auto e = translate(expr->expr, ctx);
    if (expr->type == st::UnaryExpressionType::DEREF &&
        ctx.__lvalueContext == false) {
        return ast::makeNewMemRead(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::DEREF &&
               ctx.__lvalueContext == true) {
        return ast::makeNewMemWrite(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::ADDR) {
        return ast::makeNewAddr(std::move(e));
    } else if (expr->type == st::UnaryExpressionType::NEG) {
        return ast::makeNewBinOp(ast::makeConstInt(0), std::move(e),
                                 ast::BinOpKind::Sub);
    }

    throw std::runtime_error(
        "translate(const st::UnaryExpression &expr, Ctx &ctx)");
    std::unreachable();
}

[[nodiscard]] std::unique_ptr<ast::Node> translate(
    const std::unique_ptr<st::AdditiveExpression>& expr, Ctx& ctx) {
    auto lhs = translate(expr->lhs, ctx);
    auto rhs = translate(expr->rhs, ctx);
    std::unordered_map<st::AdditiveExpressionType, ast::BinOpKind> mp = {
        {st::AdditiveExpressionType::ADD, ast::BinOpKind::Add},
        {st::AdditiveExpressionType::SUB, ast::BinOpKind::Sub},
        {st::AdditiveExpressionType::EQ, ast::BinOpKind::Eq},
        {st::AdditiveExpressionType::GT, ast::BinOpKind::Gt},
        {st::AdditiveExpressionType::LT, ast::BinOpKind::Lt},
    };
    if (mp.find(expr->type) != mp.end()) {
        return ast::makeNewBinOp(std::move(lhs), std::move(rhs),
                                 mp[expr->type]);
    }
    throw std::runtime_error(
        "translate(const st::AdditiveExpression &expr, Ctx &ctx)");
}

auto translate(const std::unique_ptr<st::ForStatement>& stmt, Ctx& ctx)
    -> std::unique_ptr<ast::Node> {
    const st::ForDeclaration& init = stmt->init;
    const auto iden =
        init.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = asttraits::toDataType(init);
    auto var = ast::makeNewVar(iden, *datatype);
    ctx.local_variables[iden] = var.get();
    const auto& expr = init.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext(
        "translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
        false);
    auto initInFirstEntryOfForLoop = translate(expr, ctx);
    ctx.set_lvalueContext(
        "translate(const std::unique_ptr<st::ForStatement> &stmt, Ctx &ctx)",
        true);
    auto forInit =
        ast::makeNewMove(std::move(var), std::move(initInFirstEntryOfForLoop));
    std::optional<std::unique_ptr<ast::Node>> forCondition;
    std::optional<std::unique_ptr<ast::Node>> forUpdate;
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
    auto result =
        ast::makeNewForLoop(std::move(forInit), std::move(forCondition),
                            std::move(forUpdate), std::move(body));
    return result;
}

auto translate(const std::unique_ptr<st::FunctionCallExpression>& expr,
               Ctx& ctx) -> std::unique_ptr<ast::Node> {
    std::vector<std::unique_ptr<ast::Node>> args;
    for (const auto& arg : expr->args) {
        auto e = translate(arg, ctx);
        args.push_back(std::move(e));
    }
    return ast::makeNewCall(expr->name, std::move(args));
}

// expression
[[nodiscard]] std::unique_ptr<ast::Node> translate(const st::Expression& expr,
                                                   Ctx& ctx) {
    return std::visit(
        [&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, expr);
}

// return statement
[[nodiscard]] std::unique_ptr<ast::Node> translate(
    const std::unique_ptr<st::ReturnStatement>& stmt, Ctx& ctx) {
    ctx.set_lvalueContext(
        "translate(const st::ReturnStatement &stmt, Ctx &ctx)", false);
    auto expr = translate(stmt->expr, ctx);
    ctx.set_lvalueContext(
        "translate(const st::ReturnStatement &stmt, Ctx &ctx)", true);
    return ast::makeNewReturn(std::move(expr));
}

// expression statement
[[nodiscard]] std::unique_ptr<ast::Node> translate(
    const std::unique_ptr<st::ExpressionStatement>& stmt, Ctx& ctx) {
    return translate(stmt->expr, ctx);
}

// selection statement statement
auto translate(const std::unique_ptr<st::SelectionStatement>& stmt, Ctx& ctx)
    -> std::unique_ptr<ast::Node> {
    auto condition = translate(stmt->cond, ctx);
    auto kind = condition->binOpKind;
    if (kind != ast::BinOpKind::Eq && kind != ast::BinOpKind::Gt) {
        condition = ast::makeNewBinOp(std::move(condition),
                                      ast::makeConstInt(0), ast::BinOpKind::Eq);
        auto then = translate(*stmt->then, ctx);
        // SWAP BECAUSE WE ARE DOING A NEQ
        if (stmt->else_) {
            auto else_ = translate(*stmt->else_, ctx);
            return ast::makeNewIfStmt(std::move(condition), std::move(else_),
                                      std::move(then));
        }
        return ast::makeNewIfStmt(std::move(condition), {}, std::move(then));
    }
    auto then = translate(*stmt->then, ctx);
    if (stmt->else_) {
        auto else_ = translate(*stmt->else_, ctx);
        return ast::makeNewIfStmt(std::move(condition), std::move(then),
                                  std::move(else_));
    }
    return ast::makeNewIfStmt(std::move(condition), std::move(then), {});
}

// declaration
[[nodiscard]] auto translate(const st::Declaration& decl, Ctx& ctx)
    -> std::unique_ptr<ast::Node> {
    const auto iden =
        decl.initDeclarator.value().declarator.directDeclarator.VariableIden();
    auto datatype = asttraits::toDataType(decl);
    auto var = ast::makeNewVar(iden, *datatype);
    ctx.local_variables[iden] = var.get();
    const auto& expr = decl.initDeclarator.value().initializer.value().expr;
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          false);
    auto init = translate(expr, ctx);
    ctx.set_lvalueContext("translate(const st::Declaration &decl, Ctx &ctx)",
                          true);
    return ast::makeNewMove(std::move(var), std::move(init));
}

[[nodiscard]] std::vector<ast::FrameParam> translate(
    st::ParamTypeList& params) {
    std::vector<ast::FrameParam> result;
    for (const auto& p : params.params) {
        const auto name = p.Name();
        const auto type = asttraits::toDataType(p);
        const auto fp = ast::FrameParam{
            .name = name,
            .type = *type,
        };
        result.push_back(fp);
    }
    return result;
}

auto translateStatement(st::Statement& stmt, Ctx& ctx)
    -> std::unique_ptr<ast::Node> {
    auto s = std::move(stmt.stmt);
    return std::visit(
        [&ctx](auto&& arg) { return translate(std::move(arg), ctx); }, s);
}

auto translate(st::CompoundStatement& stmts, Ctx& ctx)
    -> std::vector<std::unique_ptr<ast::Node>> {
    if (stmts.items.empty()) {
        return {};
    }
    std::vector<std::unique_ptr<ast::Node>> result;
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

[[nodiscard]] static std::unique_ptr<ast::Node> translate(st::FuncDef* fd,
                                                          Ctx& ctx) {
    const auto functionName = fd->Name();
    auto functionParams = fd->DirectDeclarator().params;
    auto params = translate(functionParams);
    for (const auto& p : params) {
        const auto paramName = p.name;
        const auto type = p.type;
        auto var = ast::makeNewVar(paramName, type);
        ctx.local_variables[p.name] = var.get();
    }
    std::vector<std::unique_ptr<ast::Node>> body = translate(fd->body, ctx);
    return ast::makeNewFunction(functionName, std::move(body),
                                std::move(params));
}

[[nodiscard]] static std::unique_ptr<ast::Node> translate(
    const st::ExternalDeclaration& node, Ctx& ctx) {
    const auto& nv = node.node;
    if (std::holds_alternative<std::unique_ptr<st::FuncDef>>(nv)) {
        auto funcdef = std::get<std::unique_ptr<st::FuncDef>>(nv).get();
        return translate(funcdef, ctx);
    }
    const auto& decl = std::get<st::Declaration>(nv);
    return translate(decl, ctx);
}

[[nodiscard]] std::vector<std::unique_ptr<ast::Node>> translate(
    const st::Program& program) {
    auto ctx = Ctx{
        .counter = 0,
        .local_variables = {},
    };
    std::vector<std::unique_ptr<ast::Node>> nodes;
    for (const auto& decl : program.nodes) {
        auto node = translate(decl, ctx);
        nodes.push_back(std::move(node));
    }
    return nodes;
}
