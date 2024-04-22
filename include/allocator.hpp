#pragma once

#include <concepts>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "assem.hpp"

namespace target {

struct FirstLastUse {
    std::map<int, int> firstUse = {};
    std::map<int, int> lastUse = {};
};

[[nodiscard]] auto getFirstUse(const Frame& frame) -> FirstLastUse;
[[nodiscard]] auto remap(Frame& frame)
    -> std::map<VirtualRegister, VirtualRegister>;
[[nodiscard]] auto rewrite(const std::vector<Frame>& frames)
    -> std::vector<Frame>;
}  // namespace target