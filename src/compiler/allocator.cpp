#include "../../include/compiler/allocator.hpp"

#include <concepts>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../../include/compiler/qa_x86.hpp"

namespace target {

struct AllocatorContext {
   public:
    std::set<size_t> usedRegs = {};
    std::map<Register, BaseRegister> mapping = {};

    [[nodiscard]] BaseRegister getReg() {
        for (auto [idx, reg] : general_regs | std::views::enumerate) {
            if (usedRegs.find(idx) == usedRegs.end()) {
                usedRegs.insert(idx);
                return reg;
            }
        }
        throw std::runtime_error("No free registers");
    }

    void freeReg(BaseRegister freedReg) {
        for (auto it = general_regs.begin(); it != general_regs.end(); ++it) {
            if (*it == freedReg) {
                auto index = std::distance(general_regs.begin(), it);
                usedRegs.erase(index);
                return;
            }
        }
        throw std::runtime_error("Register not found");
    }
};



auto getVirtualRegisterIDs(const Instruction &instruction) -> std::tuple<std::optional<int>, std::optional<int>> {
    const auto srcId = get_src_virtual_id_if_present(instruction); 
    const auto dstId = get_dest_virtual_id_if_present(instruction);
    return {srcId, dstId};
}

auto getFirstLastUse(const Frame& frame) -> FirstLastUse {
    std::map<int, int> firstUse = {};
    std::map<int, int> lastUse = {};
    for (auto [idx, instruction] : frame.instructions | std::views::enumerate) {
        const auto [srcId, dstId] = getVirtualRegisterIDs(instruction);
        for (const auto register_id : {srcId, dstId}) {
            if (register_id.has_value()) {
                if (firstUse.find(register_id.value()) == firstUse.end()) {
                    firstUse[register_id.value()] = idx;
                }
                lastUse[register_id.value()] = idx;
            }
        }
    }
    return {firstUse, lastUse};
}

auto remap(Frame& frame) -> std::map<VirtualRegister, VirtualRegister> {
    std::map<VirtualRegister, VirtualRegister> remappedRegisters = {};
    std::map<VirtualRegister, int> newFirstUsed = {};
    for (auto [idx, instruction] : frame.instructions | std::views::enumerate) {
        std::optional<VirtualRegister> src;
        auto register_src = get_src_register(instruction);
        if (register_src.has_value()) {
            src = *register_src;
            if (newFirstUsed.find(*src) == newFirstUsed.end()) {
                newFirstUsed[*src] = idx;
            }
        }
        const auto register_dest = get_dest_register(instruction);
        if (register_dest.has_value()) {
            const auto dest = *register_dest;
            if (!std::holds_alternative<Mov>(instruction)) {
                continue;
            }
            // if not found in newFirstused, (ie first used as a dest) then we
            // need to remap
            if (newFirstUsed.find(dest) == newFirstUsed.end()) {
                if (src.has_value()) {
                    // if src is in remapped registers, then we need to remap it
                    if (remappedRegisters.find(*src) != remappedRegisters.end()) {
                        remappedRegisters[dest] = remappedRegisters[*src];
                    } else {
                        remappedRegisters[dest] = *src;
                    }
                }
            }
        }
    }
    return remappedRegisters;
}

[[nodiscard]] Frame rewrite(const Frame& frame, AllocatorContext& ctx) {
    Frame newFrame = frame;
    auto [firstUse, lastUse] = getFirstLastUse(frame);
    auto remappedRegisters = remap(newFrame);
    for (const auto& entry : remappedRegisters) {
        const auto prev = entry.first;
        const auto newReg = entry.second;
        firstUse[newReg.id] = std::min(firstUse[newReg.id], firstUse[prev.id]);
        lastUse[newReg.id] = std::max(lastUse[newReg.id], lastUse[prev.id]);
    }
    std::vector<Instruction> newInstructions = {};
    for (auto [idx, instruction] : frame.instructions | std::views::enumerate) {
        auto operation = instruction;
        auto process_register = [&ctx, &remappedRegisters, &firstUse, &lastUse,
                                 &idx](VirtualRegister& reg) -> HardcodedRegister {
            if (remappedRegisters.find(reg) != remappedRegisters.end()) {
                reg = remappedRegisters[reg];
            }

            if (ctx.mapping.find(reg) == ctx.mapping.end() || firstUse[reg.id] == idx) {
                ctx.mapping[reg] = ctx.getReg();
            }

            if (lastUse[reg.id] <= idx) {
                ctx.freeReg(ctx.mapping[reg]);
            }

            return HardcodedRegister{ctx.mapping[reg], reg.size};
        };
        auto src_op = get_src_register(operation);
        if (src_op.has_value()) {
            auto src_reg = process_register(src_op.value());
            set_src_register(operation, src_reg);
        }
        auto dest_op = get_dest_register(operation);
        if (dest_op.has_value()) {
            auto dest_reg = process_register(dest_op.value());
            set_dest_register(operation, dest_reg);
        }
        newInstructions.push_back(operation);
    }
    newFrame.instructions = newInstructions;
    return newFrame;
}

[[nodiscard]] std::vector<Frame> rewrite(const std::vector<Frame>& frames) {
    std::vector<Frame> newFrames;
    for (const auto& frame : frames) {
        AllocatorContext ctx;
        newFrames.push_back(rewrite(frame, ctx));
    }
    return newFrames;
}
}  // namespace target
