#pragma once

#include <concepts>
#include <ostream>
#include <string>
#include <variant>

#include "../../ast/ast.hpp"
#include "../target/qa_x86.hpp"

namespace qa_ir {
    
struct Temp; 
struct Variable;
struct ConstInt;

template<typename T>
struct Immediate {
    T numerical_value;
};


using Value = std::variant<Temp, target::HardcodedRegister, Variable, Immediate<int>, Immediate<float>>;

struct Variable {
    std::string name = "";
    ast::DataType type = ast::DataType{.name = "", .size = 0};
    Value *offset = nullptr;
};

struct Temp {
    int id;
    int size;
};


std::ostream& operator<<(std::ostream& os, const Value& v);



template <typename T>
concept IsRegister = std::is_same<T, target::HardcodedRegister>::value ||
                     std::is_same<T, target::VirtualRegister>::value;

template <typename T>
concept IsIRLocation =
    std::is_same<T, qa_ir::Temp>::value || std::is_same<T, qa_ir::Variable>::value;

bool operator<(const Temp& lhs, const Temp& rhs);
std::ostream& operator<<(std::ostream& os, const Temp& temp);

bool operator<(const target::HardcodedRegister& lhs, const target::HardcodedRegister& rhs);
std::ostream& operator<<(std::ostream& os, const target::HardcodedRegister& reg);

[[nodiscard]] int SizeOf(Value v);
[[nodiscard]] int SizeOfWhatItPointsTo(Value v);

}  // namespace qa_ir