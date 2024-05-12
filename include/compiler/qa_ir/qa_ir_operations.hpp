#pragma once

#include <concepts>
#include <ostream>
#include <string>
#include <variant>

#include "qa_ir.hpp"

namespace qa_ir {
struct Label {
    std::string name;
};

std::ostream& operator<<(std::ostream& os, const Label& label);

struct Mov {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const Mov& mov);

struct Ret {
    Value value;
};

std::ostream& operator<<(std::ostream& os, const Ret& ret);

struct DefineArray {
    std::string name;
    ast::DataType type;
};

std::ostream& operator<<(std::ostream& os, const DefineArray& ret);

template <ast::BaseType T, ast::BaseType U>
struct Add {
    Value dst;

    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Add<T, U>& add);

struct Sub {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Sub& sub);

struct MovR {
    Value dst;
    target::HardcodedRegister src;
};

std::ostream& operator<<(std::ostream& os, const MovR& movr);

struct Addr {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const Addr& addr);

struct Deref {
    Value dst;
    Value src;
    int depth = 1;
};

std::ostream& operator<<(std::ostream& os, const Deref& deref);

template <ast::BaseType T, ast::BaseType U>
struct Compare {
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Compare<T, U>& cmp);

struct Equal {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const Equal& eq);

struct NotEqual {
    Value dst;
    Value left;
    Value right;
};

std::ostream& operator<<(std::ostream& os, const NotEqual& neq);

template <ast::BaseType T, ast::BaseType U>
struct GreaterThan {
    Value dst;
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const GreaterThan<T, U>& gt);

template <ast::BaseType T, ast::BaseType U>
struct LessThan {
    Value dst;
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const LessThan<T, U>& lt);

struct ConditionalJumpEqual {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpEqual& cje);

struct ConditionalJumpNotEqual {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpNotEqual& cjne);

struct ConditionalJumpGreater {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpGreater& cjg);

struct ConditionalJumpLess {
    Label trueLabel;
    Label falseLabel;
};

std::ostream& operator<<(std::ostream& os, const ConditionalJumpLess& cjl);

struct Jump {
    Label label;
};

std::ostream& operator<<(std::ostream& os, const Jump& jump);

struct Call {
    std::string name;
    std::vector<Value> args;
    Value dst;
};
std::ostream& operator<<(std::ostream& os, const Call& call);

struct LabelDef {
    Label label;
};

std::ostream& operator<<(std::ostream& os, const LabelDef& labeldef);

struct DerefStore {
    Value dst;
    Value src;
};

std::ostream& operator<<(std::ostream& os, const DerefStore& derefstore);

struct DefineStackPushed {
    std::string name;
    int size;
};

std::ostream& operator<<(std::ostream& os, const DefineStackPushed& dsp);

struct PointerOffset {
    Value dst;

    ast::DataType basisType;
    Value base;
    Value offset;
};

std::ostream& operator<<(std::ostream& os, const PointerOffset& pointer_offset);

using Operation =
    std::variant<Mov, Ret, Add<ast::BaseType::INT, ast::BaseType::INT>,
                 Add<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, Sub, MovR, Addr,
                 DefineStackPushed, Deref, Compare<ast::BaseType::INT, ast::BaseType::INT>, Jump,
                 Equal, ConditionalJumpEqual, ConditionalJumpGreater, ConditionalJumpNotEqual,
                 LabelDef, Call, DerefStore, GreaterThan<ast::BaseType::INT, ast::BaseType::INT>,
                 GreaterThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, ConditionalJumpLess,
                 NotEqual, LessThan<ast::BaseType::INT, ast::BaseType::INT>,
                 LessThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, PointerOffset, DefineArray>;

using CondJ = std::variant<ConditionalJumpEqual, ConditionalJumpGreater, ConditionalJumpNotEqual,
                           ConditionalJumpLess>;

std::ostream& operator<<(std::ostream& os, const Operation& ins);

template <typename T>
concept IsValueProducingCompare = std::is_same<T, GreaterThan<ast::BaseType::INT, ast::BaseType::INT>>::value ||
                                  std::is_same<T, LessThan<ast::BaseType::INT, ast::BaseType::INT>>::value ||
                                  std::is_same<T, Compare<ast::BaseType::INT, ast::BaseType::INT>>::value ||
                                  std::is_same<T, Equal>::value ||
                                  std::is_same<T, NotEqual>::value;
}  // namespace qa_ir