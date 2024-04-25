#pragma once
#include <vector>

#include "token.hpp"

[[nodiscard]] auto __EqualsSignLookahead(const unsigned long current,
                                         const std::vector<Token>& g_tokens) -> bool;
[[nodiscard]] auto isTypeSpecifier(const Token token) -> bool;
[[nodiscard]] auto isFuncBegin(const Token first, const Token second, const Token third) -> bool;
[[nodiscard]] auto isStmtBegin(const Token t) -> bool;