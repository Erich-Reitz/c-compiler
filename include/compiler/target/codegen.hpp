#pragma once

#include <vector>

#include "../qa_ir/assem.hpp"

namespace target {

[[nodiscard]] std::string Generate(const std::vector<target::Frame>& frames);

}