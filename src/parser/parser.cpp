#include "../../include/parser/parser.hpp"

#include <source_location>

#include "../../include/parser/syntax_utils.hpp"

#define DEBUG 0

static unsigned long current = 0;
std::vector<Token> g_tokens;

auto parser_log(const char* msg, const std::source_location loc = std::source_location::current())
    -> void {
    if (DEBUG) {
        std::cout << "parser: " << msg << " at " << loc.file_name() << ":" << loc.line() << ":"
                  << loc.column() << " current_token: " << g_tokens[current].lexeme << std::endl;
    }
}

auto peek() -> Token {
    if (current >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current];
}

Token peekn(size_t n) {
    if (current + n >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current + n];
}

Token advance() {
    if (current >= g_tokens.size()) {
        return Token{TokType::TOKEN_FEOF, ""};
    }
    return g_tokens[current++];
}

Token previous() { return g_tokens[current - 1]; }

bool match(TokType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

bool isAtEnd() { return peek().type == TokType::TOKEN_FEOF; }

void consume(TokType typ) {
    if (match(typ) == false) {
        throw std::runtime_error("Expected token of type " + std::to_string(static_cast<int>(typ)) +
                                 " found " + std::to_string(static_cast<int>(peek().type)));
    }
}

auto parseDeclarationSpecs() -> std::vector<st::DeclarationSpecifier> {
    std::vector<st::DeclarationSpecifier> declspecs;
    while (isTypeSpecifier(peek())) {
        if (peek().type == TokType::TOKEN_T_INT) {
            declspecs.push_back(st::DeclarationSpecifier{
                st::TypeSpecifier{.type = st::TypeSpecifier::Type::INT, .iden = ""}});
        } else if (peek().type == TokType::TOKEN_T_FLOAT) {
            declspecs.push_back(st::DeclarationSpecifier{
                st::TypeSpecifier{.type = st::TypeSpecifier::Type::FLOAT, .iden = ""}});
        } else {
            const auto ts = st::TypeSpecifier{
                .type = st::TypeSpecifier::Type::IDEN,
                .iden = peek().lexeme,
            };
            const auto ds = st::DeclarationSpecifier{.typespecifier = ts};
            declspecs.push_back(ds);
        }
        advance();
    }
    return declspecs;
}

std::optional<st::Pointer> parsePointer() {
    size_t count = 0;
    while (match(TokType::TOKEN_STAR)) {
        count++;
    }
    if (count == 0) {
        return std::nullopt;
    }
    return st::Pointer{.level = count};
}

std::string parseIdentifier() {
    const auto tk = peek();
    if (tk.type == TokType::TOKEN_IDENTIFIER) {
        advance();
        return tk.lexeme;
    }
    std::string msg = "expected identifier, found " + tk.lexeme;
    throw std::runtime_error(msg);
}

st::ParamTypeList parseParamTypeList() {
    std::vector<st::ParameterDeclaration> params;
    while (!match(TokType::TOKEN_RIGHT_PAREN)) {
        const auto declspecs = parseDeclarationSpecs();
        const auto decl = parseDeclarator();
        params.push_back(
            st::ParameterDeclaration{.declarationSpecifiers = declspecs, .declarator = decl});
        if (match(TokType::TOKEN_COMMA) == false) {
            break;
        }
    }
    if (match(TokType::TOKEN_RIGHT_PAREN)) {
    }
    return st::ParamTypeList{.params = params, .va_args = false};
}

// This should be more recursive than it is in parsing DirectDeclarators. But as long as it works..
st::DirectDeclarator parseDirectDeclartor() {
    auto iden = parseIdentifier();
    if (match(TokType::TOKEN_LEFT_PAREN)) {
        auto paramList = parseParamTypeList();
        auto fd = st::FunctionDirectDeclarator{
            .declarator = st::VariableDirectDeclarator{.name = iden}, .params = paramList};
        return st::DirectDeclarator{.kind = st::DeclaratorKind::FUNCTION, .declarator = fd};
    }
    // won't work for function prototypes like void f(int i, int a[static i]);
    // not looking for type qualifiers
    if (match(TokType::TOKEN_LEFT_BRACKET)) {
        parser_log("found left bracket");
        auto expr = parseAssignmentExpression();
        consume(TokType::TOKEN_RIGHT_BRACKET);
        auto ad = st::ArrayDirectDeclarator{.name = iden, .size = expr};
        return st::DirectDeclarator{.kind = st::DeclaratorKind::ARRAY, .declarator = ad};
    }

    auto vd = st::VariableDirectDeclarator{.name = iden};
    return st::DirectDeclarator{.kind = st::DeclaratorKind::VARIABLE, .declarator = vd};
}

auto parseDeclarator() -> st::Declarator {
    const auto ptr = parsePointer();
    const auto dd = parseDirectDeclartor();
    return st::Declarator{.pointer = ptr, .directDeclarator = dd};
}

auto parsePrimaryExpression() -> st::Expression {
    parser_log("parsing primary expression");
    if (peek().type == TokType::TOKEN_IDENTIFIER) {
        const auto lexeme = peek().lexeme;
        advance();
        return std::make_shared<st::PrimaryExpression>(lexeme);
    }
    if (peek().type == TokType::TOKEN_NUMBER) {
        const auto lexeme = peek().lexeme;
        advance();
        if (lexeme.find('.') != std::string::npos) {
            return std::make_shared<st::PrimaryExpression>(std::stof(lexeme));
        }

        return std::make_shared<st::PrimaryExpression>(std::stoi(lexeme));
    }
    if (peek().type == TokType::TOKEN_LEFT_PAREN) {
        consume(TokType::TOKEN_LEFT_PAREN);
        auto expr = parseExpression();
        consume(TokType::TOKEN_RIGHT_PAREN);
        return expr;
    }
    throw std::runtime_error("Expected primary expression found " + peek().lexeme);
}

auto parsePostfixExpression() -> st::Expression {
    parser_log("parsing postfix expression");
    auto primary = parsePrimaryExpression();
    // function call
    if (match(TokType::TOKEN_LEFT_PAREN)) {
        std::vector<st::Expression> args;
        while (!match(TokType::TOKEN_RIGHT_PAREN)) {
            auto expr = parseExpression();
            args.push_back(std::move(expr));
            if (match(TokType::TOKEN_COMMA) == false) {
                break;
            }
        }
        if (match(TokType::TOKEN_RIGHT_PAREN)) {
        }
        const auto primary_expr = std::get<std::shared_ptr<st::PrimaryExpression>>(primary).get();
        const auto name = primary_expr->idenValue;
        return std::make_shared<st::FunctionCallExpression>(name, std::move(args));
    }

    // left hand side of a[3] = 5;
    if (match(TokType::TOKEN_LEFT_BRACKET)) {
        parser_log("parsePostfixExpression(): found left bracket");
        auto expr = parseExpression();
        const std::string name =
            std::get<std::shared_ptr<st::PrimaryExpression>>(primary)->idenValue;
        parser_log("parsePostfixExpression(): parsed expr");
        consume(TokType::TOKEN_RIGHT_BRACKET);
        return std::make_shared<st::ArrayAccessExpression>(name, std::move(expr));
    }

    return primary;
}

st::Expression parseUnaryExpression() {
    if (match(TokType::TOKEN_STAR)) {
        auto expr = parseUnaryExpression();
        return std::make_shared<st::UnaryExpression>(st::UnaryExpressionType::DEREF,
                                                     std::move(expr));
    }
    if (match(TokType::TOKEN_AMPERSAND)) {
        auto expr = parseUnaryExpression();
        return std::make_shared<st::UnaryExpression>(st::UnaryExpressionType::ADDR,
                                                     std::move(expr));
    }
    if (match(TokType::TOKEN_MINUS)) {
        auto expr = parseUnaryExpression();
        return std::make_shared<st::UnaryExpression>(st::UnaryExpressionType::NEG, std::move(expr));
    }
    return parsePostfixExpression();
}

st::Expression parseMultiplicativeExpression() {
    auto lhs = parseUnaryExpression();
    while (match(TokType::TOKEN_STAR) || match(TokType::TOKEN_SLASH)) {
        auto op_token = previous();
        auto op = st::MultiplicativeExpressionType::Mult;
        if (op_token.type == TokType::TOKEN_SLASH) {
            op = st::MultiplicativeExpressionType::Div;
        }
        auto rhs = parseUnaryExpression();
        lhs = std::make_shared<st::MultiplicativeExpression>(std::move(lhs), std::move(rhs), op);
    }
    return lhs;
}

st::Expression parseAdditiveExpression() {
    auto lhs = parseMultiplicativeExpression();
    while (match(TokType::TOKEN_PLUS) || match(TokType::TOKEN_MINUS)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::ADD;
        if (op_token.type == TokType::TOKEN_MINUS) {
            op = st::AdditiveExpressionType::SUB;
        }
        auto rhs = parseMultiplicativeExpression();
        lhs = std::make_shared<st::AdditiveExpression>(std::move(lhs), std::move(rhs), op);
    }
    return lhs;
}

st::Expression parseRelationalExpression() {
    parser_log("parsing relational expression");
    auto lhs = parseAdditiveExpression();
    if (match(TOKEN_GREATER)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::GT;
        auto rhs = parseAdditiveExpression();
        return std::make_shared<st::AdditiveExpression>(std::move(lhs), std::move(rhs), op);
    }
    if (match(TOKEN_LESS)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::LT;
        auto rhs = parseAdditiveExpression();
        return std::make_shared<st::AdditiveExpression>(std::move(lhs), std::move(rhs), op);
    }
    return lhs;
}

st::Expression parseEqualityExpression() {
    parser_log("parsing equality expression");
    auto lhs = parseRelationalExpression();
    if (match(TokType::TOKEN_EQUAL_EQUAL) || match(TokType::TOKEN_BANG_EQUAL)) {
        auto op_token = previous();
        auto op = st::AdditiveExpressionType::EQ;
        if (op_token.type == TokType::TOKEN_BANG_EQUAL) {
            op = st::AdditiveExpressionType::NEQ;
        }
        auto rhs = parseRelationalExpression();
        return std::make_shared<st::AdditiveExpression>(std::move(lhs), std::move(rhs), op);
    }
    return lhs;
}

st::Expression parseAssignmentExpression() {
    if (__EqualsSignLookahead(current, g_tokens) == false) {
        parser_log("parsing equality expression");
        return parseEqualityExpression();
    }
    auto lhs = parseEqualityExpression();
    parser_log("parseAssignmentExpression(): parsed lhs");
    consume(TokType::TOKEN_EQUAL);
    auto rhs = parseEqualityExpression();
    return std::make_shared<st::AssignmentExpression>(std::move(lhs), std::move(rhs));
}

auto parseExpression() -> st::Expression { return parseAssignmentExpression(); }

auto parseReturnStatement() -> std::shared_ptr<st::ReturnStatement> {
    auto expr = parseExpression();
    consume(TokType::TOKEN_SEMICOLON);
    return std::make_shared<st::ReturnStatement>(std::move(expr));
}

auto parseExpressionStatement() -> std::shared_ptr<st::ExpressionStatement> {
    parser_log("parsing expression statement");
    auto expr = parseExpression();
    parser_log("parseExpressionStatement(): parsed expression");
    consume(TokType::TOKEN_SEMICOLON);
    return std::make_shared<st::ExpressionStatement>(std::move(expr));
}

std::shared_ptr<st::SelectionStatement> parseIfStatement() {
    consume(TokType::TOKEN_LEFT_PAREN);
    auto expr = parseExpression();
    consume(TokType::TOKEN_RIGHT_PAREN);
    auto thenStmt = parseCompoundStatement();
    auto thenStmtUnique = std::make_shared<st::CompoundStatement>(std::move(thenStmt));
    if (match(TokType::TOKEN_ELSE)) {
        auto elseStmt = parseCompoundStatement();
        auto elseStmtUnique = std::make_shared<st::CompoundStatement>(std::move(elseStmt));
        return std::make_shared<st::SelectionStatement>(std::move(expr), std::move(thenStmtUnique),
                                                        std::move(elseStmtUnique));
    }
    return std::make_shared<st::SelectionStatement>(std::move(expr), std::move(thenStmtUnique),
                                                    nullptr);
}

auto parseForDeclaration() -> st::ForDeclaration {
    auto declspecs = parseDeclarationSpecs();
    auto decl = parseInitDeclarator();
    return st::ForDeclaration(declspecs, std::move(decl));
}

auto parseForStatement() -> std::shared_ptr<st::ForStatement> {
    consume(TokType::TOKEN_LEFT_PAREN);
    // if the next thing is a declaration specifier then we want to parse a
    // forDeclaration. else we want to parse an expression
    st::ForDeclaration decl({}, {});
    std::optional<st::Expression> cond = std::nullopt;
    std::optional<st::Expression> inc = std::nullopt;
    if (isTypeSpecifier(peek())) {
        // should parse the semicolon
        decl = parseForDeclaration();
        if (peek().type != TokType::TOKEN_SEMICOLON) {
            cond = parseExpression();
            consume(TokType::TOKEN_SEMICOLON);
        } else {
            consume(TokType::TOKEN_SEMICOLON);
        }
        if (peek().type != TokType::TOKEN_RIGHT_PAREN) {
            inc = parseExpression();
        }
    } else {
        throw std::runtime_error("Expected declaration specifier found " + peek().lexeme);
    }
    consume(TokType::TOKEN_RIGHT_PAREN);
    auto body = parseCompoundStatement();
    auto bodyUnique = std::make_shared<st::CompoundStatement>(std::move(body));
    return std::make_shared<st::ForStatement>(std::move(decl), std::move(cond), std::move(inc),
                                              std::move(bodyUnique));
}

/**
 *  Question:
 *  Can ParseStatement() yield CompoundStatement?
 *  Answer:
 *      Not straight up. Needs to be requested from things like
 *        parseIfStatement() or parseForStatement()
 **/
auto parseStatement() -> st::Statement {
    if (match(TokType::TOKEN_RETURN)) {
        auto ret = parseReturnStatement();
        return st::Statement(std::move(ret));
    }
    if (match(TokType::TOKEN_IF)) {
        auto ifStmt = parseIfStatement();
        return st::Statement(std::move(ifStmt));
    }
    if (match(TokType::TOKEN_FOR)) {
        auto forStmt = parseForStatement();
        return st::Statement(std::move(forStmt));
    }
    auto expr = parseExpressionStatement();
    return st::Statement(std::move(expr));
}

st::BlockItem parseBlockItem() {
    if (isStmtBegin(peek())) {
        auto item = parseStatement();
        return st::BlockItem(std::move(item));
    }
    auto decl = parseDeclaration();
    return st::BlockItem(std::move(decl));
}

st::CompoundStatement parseCompoundStatement() {
    // left
    consume(TokType::TOKEN_LEFT_BRACE);
    std::vector<st::BlockItem> blockItems;
    while (!match(TokType::TOKEN_RIGHT_BRACE)) {
        auto bi = parseBlockItem();
        blockItems.push_back(std::move(bi));
    }
    return st::CompoundStatement{.items = std::move(blockItems)};
}

st::Initalizer parseInitalizer() {
    auto expr = parseExpression();
    return st::Initalizer(std::move(expr));
}

st::InitDeclarator parseInitDeclarator() {
    auto declarator = parseDeclarator();
    if (match(TokType::TOKEN_EQUAL)) {
        auto initializer = parseInitalizer();
        consume(TokType::TOKEN_SEMICOLON);
        return st::InitDeclarator{.declarator = declarator, .initializer = std::move(initializer)};
    }
    consume(TokType::TOKEN_SEMICOLON);
    return st::InitDeclarator{.declarator = declarator, .initializer = std::nullopt};
}

st::Declaration parseDeclaration() {
    const auto declspecs = parseDeclarationSpecs();
    // hack obvs
    auto initDeclarator = parseInitDeclarator();
    return st::Declaration{.declarationSpecifiers = declspecs,
                           .initDeclarator = std::move(initDeclarator)};
}

std::shared_ptr<st::FuncDef> parseFunctionDefinition() {
    const auto declspecs = parseDeclarationSpecs();
    const auto decl = parseDeclarator();
    st::CompoundStatement body = parseCompoundStatement();
    return std::make_shared<st::FuncDef>(declspecs, decl, std::move(body));
}

std::optional<st::ExternalDeclaration> parseExternalDeclaration() {
    const auto nxt = peek();
    if (nxt.type == TokType::TOKEN_SEMICOLON) {
        advance();
        return std::nullopt;
    }
    if (isFuncBegin(peek(), peekn(1), peekn(2))) {
        auto fd = parseFunctionDefinition();
        return st::ExternalDeclaration(std::move(fd));
    }
    auto decl = parseDeclaration();
    return st::ExternalDeclaration(std::move(decl));
}

st::Program parse(const std::vector<Token>& tokens) {
    g_tokens = tokens;
    std::vector<st::ExternalDeclaration> nodes;
    while (isAtEnd() == false) {
        auto ed = parseExternalDeclaration();
        if (ed.has_value()) {
            auto decl = std::move(ed.value());
            nodes.push_back(std::move(decl));
        }
    }
    return st::Program(std::move(nodes));
}