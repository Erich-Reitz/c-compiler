#pragma once

#include "qa_x86_instructions.hpp"
#include "qa_x86_registers.hpp"

#include <concepts>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>



namespace target {

int sixteenByteAlign(int size) ; 


std::optional<int> get_src_virtual_id_if_present(const Instruction& ins);
std::optional<int> get_dest_virtual_id_if_present(const Instruction& ins);
std::optional<VirtualRegister> get_src_register(const Instruction& ins);
std::optional<VirtualRegister> get_dest_register(const Instruction& ins);

auto set_src_register(Instruction& ins, Register reg) -> void;
auto set_dest_register(Instruction& ins, Register reg) -> void;

template <typename T>
concept HasRegisterSrc = requires(T t) {
    { t.src } -> std::convertible_to<Register>;
};

template <typename T>
concept HasRegisterDest = requires(T t) {
    { t.dst } -> std::convertible_to<Register>;
};



}  // namespace target