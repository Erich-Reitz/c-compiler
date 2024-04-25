#pragma once

#include <vector>

#include "st.hpp"
#include "token.hpp"

[[nodiscard]] auto parseDirectDeclartor() -> st::DirectDeclarator;
[[nodiscard]] auto parseDeclaration() -> st::Declaration;
[[nodiscard]] auto parseDeclarator() -> st::Declarator;
[[nodiscard]] auto parseCompoundStatement() -> st::CompoundStatement;
[[nodiscard]] auto parseExpression() -> st::Expression;
[[nodiscard]] auto peek() -> Token;
[[nodiscard]] auto peekn(size_t n) -> Token;
auto advance() -> Token;
[[nodiscard]] auto previous() -> Token;
[[nodiscard]] auto match(TokType type) -> bool;
[[nodiscard]] auto isAtEnd() -> bool;

auto consume(TokType typ) -> void;

[[nodiscard]] auto parseDeclarationSpecs() -> std::vector<st::DeclarationSpecifier>;
[[nodiscard]] auto parsePointer() -> std::optional<st::Pointer>;
[[nodiscard]] auto parseIdentifier() -> std::string;
[[nodiscard]] auto parseParamTypeList() -> st::ParamTypeList;

[[nodiscard]] auto parsePrimaryExpression() -> st::Expression;
[[nodiscard]] auto parseReturnStatement() -> std::unique_ptr<st::ReturnStatement>;
[[nodiscard]] auto parseExpressionStatement() -> std::unique_ptr<st::ExpressionStatement>;
[[nodiscard]] auto parseStatement() -> st::Statement;
[[nodiscard]] auto parseIfStatement() -> std::unique_ptr<st::SelectionStatement>;
[[nodiscard]] auto parseBlockItem() -> st::BlockItem;
[[nodiscard]] auto parseInitalizer() -> st::Initalizer;
[[nodiscard]] auto parseInitDeclarator() -> st::InitDeclarator;
[[nodiscard]] auto parseFunctionDefinition() -> std::unique_ptr<st::FuncDef>;
[[nodiscard]] auto parseExternalDeclaration() -> std::optional<st::ExternalDeclaration>;
[[nodiscard]] auto parse(const std::vector<Token>& tokens) -> st::Program;
[[nodiscard]] auto parsePostfixExpression() -> st::Expression;
[[nodiscard]] auto parseUnaryExpression() -> st::Expression;
[[nodiscard]] auto parseAdditiveExpression() -> st::Expression;
[[nodiscard]] auto parseRelationalExpression() -> st::Expression;
[[nodiscard]] auto parseEqualityExpression() -> st::Expression;
[[nodiscard]] auto parseAssignmentExpression() -> st::Expression;
[[nodiscard]] auto parseForStatement() -> std::unique_ptr<st::ForStatement>;
[[nodiscard]] auto parseForDeclaration() -> st::ForDeclaration;

[[nodiscard]] st::Program parse(const std::vector<Token>& tokens);