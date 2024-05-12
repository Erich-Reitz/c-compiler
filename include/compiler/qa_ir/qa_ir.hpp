#pragma once

#include <concepts>
#include <ostream>
#include <string>
#include <variant>

#include "../../ast/ast.hpp"
#include "../target/qa_x86_registers.hpp"

namespace qa_ir {

struct Temp;
struct Variable;
struct ConstInt;

template <typename T>
struct Immediate {
    T numerical_value;
};

using Value =
    std::variant<Temp, target::HardcodedRegister, Variable, Immediate<int>, Immediate<float>>;

[[nodiscard]] ast::DataType ResultingTypeForBinOp(ast::DataType lhs, ast::DataType rhs,
                                                  ast::BinOpKind op);

[[nodiscard]] ast::DataType GetDataType(Value v);

struct Variable {
    std::string name = "";
    ast::DataType type = ast::DataType{.base_type = ast::BaseType::NONE};

    [[nodiscard]] auto is_immediate_float() const -> bool { return type.is_float(); };

    [[nodiscard]] auto is_immediate_int() const -> bool { return type.is_int(); };

    [[nodiscard]] auto is_int_ptr() const -> bool { return type.is_int_ptr(); };

    [[nodiscard]] auto is_float_ptr() const -> bool { return type.is_float_ptr(); };
};

struct Temp {
    int id;
    ast::DataType type = ast::DataType{.base_type = ast::BaseType::NONE};

    Temp(int p_id, ast::DataType p_type) : id(p_id), type(p_type) {}
};

std::ostream& operator<<(std::ostream& os, const Value& v);

template <typename T>
concept IsRegister =
    std::is_same<T, target::HardcodedRegister>::value ||
    std::is_same<T, target::VirtualRegister>::value || std::is_same<T, target::Register>::value;

template <typename T>
concept IsEphemeral = IsRegister<T> || std::is_same<T, Temp>::value;

template <typename T>
concept IsIRLocation = std::is_same<T, qa_ir::Variable>::value;

template <typename T>
concept IsImmediate =
    std::is_same<T, Immediate<int>>::value || std::is_same<T, Immediate<float>>::value;

bool operator<(const Temp& lhs, const Temp& rhs);
std::ostream& operator<<(std::ostream& os, const Temp& temp);

bool operator<(const target::HardcodedRegister& lhs, const target::HardcodedRegister& rhs);
std::ostream& operator<<(std::ostream& os, const target::HardcodedRegister& reg);

[[nodiscard]] int SizeOf(Value v);

}  // namespace qa_ir