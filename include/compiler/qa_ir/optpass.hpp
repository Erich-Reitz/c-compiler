#pragma once

#include <vector>

#include "assem.hpp"

namespace qa_ir {
std::vector<Frame> move_from_temp_dest_pass(const std::vector<Frame>& frames);
}