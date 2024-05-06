#pragma once

#include <string>
#include <vector>

#include "qa_x86_frame.hpp"

namespace target {

[[nodiscard]] std::string Generate(const std::vector<target::Frame>& frames);

}