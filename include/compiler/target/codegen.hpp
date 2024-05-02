#pragma once

#include "qa_x86_frame.hpp"

#include <string>
#include <vector>



namespace target {


[[nodiscard]] std::string Generate(const std::vector<target::Frame>& frames);

}