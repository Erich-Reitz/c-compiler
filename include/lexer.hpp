#pragma once

#include <string>
#include <vector>

#include "token.hpp"
namespace lexer {
[[nodiscard]] auto peek() -> char;
[[nodiscard]] auto isAtEnd() -> bool;
auto advance() -> char;
[[nodiscard]] auto lex(const std::string& source) -> std::vector<Token>;
}  // namespace lexer
