#pragma once

#include <vector>

#include "assem.hpp"

namespace codegen {
[[nodiscard]] std::string Generate(const std::vector<target::Frame>& frames);
}