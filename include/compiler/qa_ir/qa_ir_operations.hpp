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

template <ast::BaseType T, ast::BaseType U>
struct Sub {
    Value dst;
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Sub<T, U>& sub);

template <ast::BaseType T, ast::BaseType U>
struct Mult {
    Value dst;

    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Mult<T, U>& add);

template <ast::BaseType T, ast::BaseType U>
struct Div {
    Value dst;

    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Div<T, U>& add);

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

template <ast::BaseType T, ast::BaseType U>
struct Equal {
    Value dst;
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const Equal<T, U>& eq);

template <ast::BaseType T, ast::BaseType U>
struct NotEqual {
    Value dst;
    Value left;
    Value right;
};

template <ast::BaseType T, ast::BaseType U>
std::ostream& operator<<(std::ostream& os, const NotEqual<T, U>& neq);

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

using Operation = std::variant<
    Mov, Ret, Add<ast::BaseType::INT, ast::BaseType::INT>,
    Add<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, Sub<ast::BaseType::INT, ast::BaseType::INT>,
    MovR, Addr, Sub<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, DefineStackPushed, Deref,
    Compare<ast::BaseType::INT, ast::BaseType::INT>, Jump,
    Equal<ast::BaseType::INT, ast::BaseType::INT>,
    Equal<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, ConditionalJumpEqual, ConditionalJumpGreater,
    ConditionalJumpNotEqual, LabelDef, Call, DerefStore,
    GreaterThan<ast::BaseType::INT, ast::BaseType::INT>,
    GreaterThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>, ConditionalJumpLess,
    NotEqual<ast::BaseType::INT, ast::BaseType::INT>,
    NotEqual<ast::BaseType::FLOAT, ast::BaseType::FLOAT>,
    LessThan<ast::BaseType::INT, ast::BaseType::INT>,
    LessThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>,
    Mult<ast::BaseType::INT, ast::BaseType::INT>, Mult<ast::BaseType::FLOAT, ast::BaseType::FLOAT>,
    PointerOffset, DefineArray, Div<ast::BaseType::INT, ast::BaseType::INT>,
    Div<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>;

using CondJ = std::variant<ConditionalJumpEqual, ConditionalJumpGreater, ConditionalJumpNotEqual,
                           ConditionalJumpLess>;

std::ostream& operator<<(std::ostream& os, const Operation& ins);

template <typename O>
concept HasIRDestination = requires(O o) {
    { o.dst } -> std::convertible_to<Value>;
};

template <typename O>
concept IsArthOverIntegers =
    std::is_same<O, qa_ir::Add<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::Sub<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::Mult<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::Div<ast::BaseType::INT, ast::BaseType::INT>>::value;

template <typename O>
concept IsCommunativeOperationOverIntegers =
    std::is_same<O, qa_ir::Add<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::Mult<ast::BaseType::INT, ast::BaseType::INT>>::value;

template <typename O>
concept IsSubtractionOfIntegers =
    std::is_same<O, qa_ir::Sub<ast::BaseType::INT, ast::BaseType::INT>>::value;

template <typename O>
concept IsArthOverFloats =
    std::is_same<O, qa_ir::Add<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::Sub<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::Mult<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::Div<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value;

template <typename O>
concept IsValueProducingCompareOverIntegers =
    std::is_same<O, qa_ir::GreaterThan<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::LessThan<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::Equal<ast::BaseType::INT, ast::BaseType::INT>>::value ||
    std::is_same<O, qa_ir::NotEqual<ast::BaseType::INT, ast::BaseType::INT>>::value;

template <typename O>
concept IsCompareOverIntegers =
    IsValueProducingCompareOverIntegers<O> ||
    std::is_same<O, qa_ir::Compare<ast::BaseType::INT, ast::BaseType::INT>>::value;

template <typename O>
concept IsValueProducingCompareOverFloats =
    std::is_same<O, qa_ir::GreaterThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::LessThan<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::Equal<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value ||
    std::is_same<O, qa_ir::NotEqual<ast::BaseType::FLOAT, ast::BaseType::FLOAT>>::value;

}  // namespace qa_ir