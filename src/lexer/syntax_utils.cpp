#include "../../include/parser/syntax_utils.hpp"

#include <set>

auto isTypeSpecifier(const Token token) -> bool {
    return token.type == TokType::TOKEN_T_INT || token.type == TokType::TOKEN_T_VOID;
}

auto isStmtBegin(const Token t) -> bool {
    std::set<TokType> stmtBeginSet = {
        TokType::TOKEN_RETURN, TokType::TOKEN_IDENTIFIER, TokType::TOKEN_STAR,
        TokType::TOKEN_IF,     TokType::TOKEN_FOR,
    };
    return stmtBeginSet.find(t.type) != stmtBeginSet.end();
}

auto isFuncBegin(const Token first, const Token second, const Token third) -> bool {
    return isTypeSpecifier(first) && second.type == TokType::TOKEN_IDENTIFIER &&
           third.type == TokType::TOKEN_LEFT_PAREN;
}

auto __EqualsSignLookahead(const unsigned long current, const std::vector<Token>& g_tokens)
    -> bool {
    unsigned long i = current;
    unsigned margin = 0;
    while (i < g_tokens.size()) {
        if (g_tokens[i].type == TokType::TOKEN_EQUAL) {
            return true;
        }
        if (g_tokens[i].type == TokType::TOKEN_SEMICOLON) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_LEFT_BRACE) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_RIGHT_BRACE) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_LEFT_PAREN) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_RIGHT_PAREN) {
            return false;
        }
        if (g_tokens[i].type == TokType::TOKEN_LEFT_BRACKET) {
            margin++;
        }
        if (g_tokens[i].type == TokType::TOKEN_RIGHT_BRACKET) {
            if (margin == 0) {
                return false;
            }
            margin--;
        }
        i++;
    }
    return false;
}