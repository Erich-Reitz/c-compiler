#pragma once

#include <iostream>
#include <optional>
#include <variant>

#include "location.hpp"

namespace qa_ir {

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
    HardcodedRegister src;
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

struct Call {
    std::string name;
    std::vector<Value> args;
    Value dst;
};

std::ostream &operator<<(std::ostream &os, const Label &label);

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
    std::variant<Mov, Ret, Add, Sub, MovR,Addr, DefineStackPushed, Deref, StoreAddr, Compare,
    Equal, ConditionalJumpEqual,ConditionalJumpGreater, LabelDef, Call, DerefStore, GreaterThan>;

using CondJ =
    std::variant<ConditionalJumpEqual, ConditionalJumpGreater>;

Label get_true_label(const CondJ &condj);
Label get_false_label(const CondJ &condj);

std::ostream &operator<<(std::ostream &os, const Operation &ins);

} // namespace qa_ir