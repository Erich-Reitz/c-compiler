#include "../../../include/compiler/qa_ir/optpass.hpp"

#include <ranges>
#include <unordered_map>
#include <variant>

namespace qa_ir {

template <typename V>
    requires HasIRDestination<V>
std::optional<Value> get_destination(const V& v) {
    return v.dst;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template <typename V>
std::optional<Value> get_destination(const V& v) {
    return std::nullopt;
}
#pragma GCC diagnostic pop

template <typename V>
    requires HasIRDestination<V>
Operation set_destination(V& v, const Value& new_dest) {
    v.dst = new_dest;
    return v;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template <typename V>
Operation set_destination(V& v, const Value& new_dest) {
    throw("nope");
}
#pragma GCC diagnostic pop

Frame move_from_temp_dest_pass(const Frame& frame) {
    std::vector<Operation> new_instructions;
    std::vector<std::pair<qa_ir::Temp, int>> idx_where_used_as_dest;
    int skipped = 0;
    for (const auto [idx, ins] : std::views::enumerate(frame.instructions)) {
        bool do_not_add_current = false;
        if (auto dest = std::visit([](auto&& arg) { return get_destination(arg); }, ins);
            dest.has_value() && std::holds_alternative<Temp>(*dest)) {
            idx_where_used_as_dest.push_back({std::get<Temp>(*dest), idx});
        }

        if (std::holds_alternative<Mov>(ins)) {
            const auto& mov = std::get<Mov>(ins);
            if (std::holds_alternative<Temp>(mov.src)) {
                const auto& mov_src = std::get<Temp>(mov.src);
                const auto& mov_dest = mov.dst;
                for (auto& [temp, j_idx] : idx_where_used_as_dest) {
                    if (temp.id == mov_src.id) {
                        auto previous_instruction = frame.instructions[j_idx];
                        auto new_instruction = std::visit(
                            [&mov_dest](auto&& arg) { return set_destination(arg, mov_dest); },
                            previous_instruction);
                        new_instructions[j_idx - skipped] = new_instruction;
                        do_not_add_current = true;
                    }
                }
            }
        }
        if (!do_not_add_current) {
            new_instructions.push_back(ins);
        } else {
            skipped++;
        }
    }
    return Frame{.name = frame.name, .instructions = new_instructions, .size = frame.size};
}

std::vector<Frame> move_from_temp_dest_pass(const std::vector<Frame>& frames) {
    std::vector<Frame> new_frames;
    for (const auto& frame : frames) {
        const auto new_frame = move_from_temp_dest_pass(frame);
        new_frames.push_back(new_frame);
    }
    return new_frames;
}
}  // namespace qa_ir