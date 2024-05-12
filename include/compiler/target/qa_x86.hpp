#pragma once

#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../../include/ast/DataType.hpp"
#include "qa_x86_instructions.hpp"
#include "qa_x86_locations.hpp"
#include "qa_x86_registers.hpp"

namespace target {
[[nodiscard]] int SizeOf(const Location& loc);
int sixteenByteAlign(int size);

template <typename T>
concept IsX86Instruction = std::derived_from<T, x86Instruction>;

// requires IsX86Instruction<T>
template <typename T>
concept InstructionWithImmediateRegisterSource = IsX86Instruction<T> && requires(T t) {
    { t.src } -> std::convertible_to<Register>;
};

template <typename T>
concept InstructionWithComputedRegisterSource = IsX86Instruction<T> && requires(T t) {
    { t.src.src } -> std::convertible_to<Register>;
};

template <typename T>
concept InstructionWithImmediateRegisterDest = IsX86Instruction<T> && requires(T t) {
    { t.dst } -> std::convertible_to<Register>;
};

template <typename T>
concept InstructionWithComputedRegisterDest = IsX86Instruction<T> && requires(T t) {
    { t.dst.src } -> std::convertible_to<Register>;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template <typename T>
    requires InstructionWithComputedRegisterSource<T>
std::optional<VirtualRegister> get_src_register(T& ins) {
    if (std::holds_alternative<VirtualRegister>(ins.src.src)) {
        return std::get<VirtualRegister>(ins.src.src);
    }
    return std::nullopt;
}

template <typename T>
    requires InstructionWithImmediateRegisterSource<T>
std::optional<VirtualRegister> get_src_register(T& ins) {
    if (std::holds_alternative<VirtualRegister>(ins.src)) {
        return std::get<VirtualRegister>(ins.src);
    }
    return std::nullopt;
}

std::optional<VirtualRegister> get_src_register(IsX86Instruction auto& ins) { return std::nullopt; }

template <typename T>
    requires InstructionWithComputedRegisterDest<T>
std::optional<VirtualRegister> get_dest_register(T& ins) {
    if (std::holds_alternative<VirtualRegister>(ins.dst.src)) {
        return std::get<VirtualRegister>(ins.dst.src);
    }
    return std::nullopt;
}

template <typename T>
    requires InstructionWithImmediateRegisterDest<T>
std::optional<VirtualRegister> get_dest_register(T& ins) {
    if (std::holds_alternative<VirtualRegister>(ins.dst)) {
        return std::get<VirtualRegister>(ins.dst);
    }
    return std::nullopt;
}

std::optional<VirtualRegister> get_dest_register(IsX86Instruction auto& ins) {
    return std::nullopt;
}

std::optional<int> src_register_id(IsX86Instruction auto& ins) {
    const auto reg = get_src_register(ins);
    if (reg.has_value()) {
        return reg->id;
    }
    return std::nullopt;
}

std::optional<int> dest_register_id(IsX86Instruction auto& ins) {
    auto reg = get_dest_register(ins);
    if (reg.has_value()) {
        return reg->id;
    }
    return std::nullopt;
}

template <typename T>
    requires InstructionWithImmediateRegisterSource<T>
void set_src_register(T& ins, Register reg) {
    ins.src = reg;
}

template <typename T>
    requires InstructionWithComputedRegisterSource<T>
void set_src_register(T& ins, Register reg) {
    ins.src.src = reg;
}

void set_src_register(IsX86Instruction auto& ins, Register reg) {
    throw std::runtime_error(
        "set_src_register called on an instruction that does not have a src register");
}

template <typename T>
    requires InstructionWithImmediateRegisterDest<T>
void set_dest_register(T& ins, Register reg) {
    ins.dst = reg;
}

template <typename T>
    requires InstructionWithComputedRegisterDest<T>
void set_dest_register(T& ins, Register reg) {
    ins.dst.src = reg;
}

void set_dest_register(IsX86Instruction auto& ins, Register reg) {
    throw std::runtime_error(
        "set_dest_register called on an instruction that does not have a src register");
}

#pragma GCC diagnostic pop
}  // namespace target