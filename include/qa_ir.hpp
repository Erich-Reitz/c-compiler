#pragma once

#include <concepts>
#include <ostream>
#include <string>
#include <variant>

#include "ast.hpp"
#include "qa_x86.hpp"

namespace qa_ir {
struct Variable {
    std::string name;
    int version = 0;
    int size = 0;
};

struct Temp {
    int id;
    int size;
};

using Value = std::variant<Temp, target::HardcodedRegister, Variable, int>;

struct Label {
    std::string name;
};

struct Mov {
    Value dst;
    Value src;
};

struct Ret {
    Value value;
};

struct Add {
    Value dst;

    Value left;
    Value right;
};

struct Sub {
    Value dst;
    Value left;
    Value right;
};

struct MovR {
    Value dst;
    target::HardcodedRegister src;
};

struct Addr {
    Value dst;
    Value src;
};

struct Deref {
    Value dst;
    Value src;
    int depth = 1;
};

struct StoreAddr {
    Value dst;
    Value src;
};

struct Compare {
    Value left;
    Value right;
};

struct Equal {
    Value dst;
    Value left;
    Value right;
};

struct GreaterThan {
    Value dst;
    Value left;
    Value right;
};

struct ConditionalJumpEqual {
    Label trueLabel;
    Label falseLabel;
};

struct ConditionalJumpGreater {
    Label trueLabel;
    Label falseLabel;
};

struct ConditionalJumpLess {
    Label trueLabel;
    Label falseLabel;
};

struct Jump {
    Label label;
};

struct Call {
    std::string name;
    std::vector<Value> args;
    Value dst;
};

std::ostream& operator<<(std::ostream& os, const Label& label);

struct LabelDef {
    Label label;
};

struct DerefStore {
    Value dst;
    Value src;
};

struct DefineStackPushed {
    std::string name;
    int size;
};

using Operation =
    std::variant<Mov, Ret, Add, Sub, MovR, Addr, DefineStackPushed, Deref,
                 Compare, Jump, Equal, ConditionalJumpEqual,
                 ConditionalJumpGreater, LabelDef, Call, DerefStore,
                 GreaterThan, ConditionalJumpLess>;

using CondJ = std::variant<ConditionalJumpEqual, ConditionalJumpGreater>;

Label get_true_label(const CondJ& condj);
Label get_false_label(const CondJ& condj);

std::ostream& operator<<(std::ostream& os, const Operation& ins);

template <typename T>
concept Integral = std::is_integral<T>::value;

template <typename T>
concept IsRegister = std::is_same<T, target::HardcodedRegister>::value ||
                     std::is_same<T, target::VirtualRegister>::value;

template <typename T>
concept IsIRLocation = std::is_same<T, qa_ir::Temp>::value ||
                       std::is_same<T, qa_ir::Variable>::value;

bool operator<(const Temp& lhs, const Temp& rhs);
std::ostream& operator<<(std::ostream& os, const Temp& temp);

bool operator<(const target::HardcodedRegister& lhs,
               const target::HardcodedRegister& rhs);
std::ostream& operator<<(std::ostream& os,
                         const target::HardcodedRegister& reg);

std::ostream& operator<<(std::ostream& os, const Value& v);

[[nodiscard]] int SizeOf(Value v);
[[nodiscard]] int SizeOfWhatItPointsTo(Value v);

}  // namespace qa_ir