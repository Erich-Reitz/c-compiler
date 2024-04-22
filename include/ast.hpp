#pragma once
#include <memory>
#include <string>
#include <vector>

#include "st.hpp"

namespace ast {
enum class NodeType {
    ConstInt,
    Frame,
    Move,
    Temp,
    Seq,
    Jump,
    Return,
    Var,
    Deref,
    Addr,
    BinOp,
    If,
    Call,
    ForLoop
};

enum class BinOpKind { Add, Sub, Eq, Gt, Lt };

[[nodiscard]] auto is_arithmetic(BinOpKind kind) -> bool;
[[nodiscard]] auto is_comparison(BinOpKind kind) -> bool;

enum class SelectionKind { If };
struct DataType {
    std::string name;
    int size = 0;
    DataType* pointsTo = nullptr;

    DataType() : name(""), size(0), pointsTo(nullptr) {}

    DataType(std::string name, int size, DataType* pointsTo)
        : name(name), size(size), pointsTo(pointsTo) {}

    DataType(const DataType& other)
        : name(other.name), size(other.size), pointsTo(nullptr) {
        if (other.pointsTo != nullptr) {
            pointsTo = new DataType(*other.pointsTo);
        }
    }

    DataType& operator=(const DataType& other) {
        if (this != &other) {
            name = other.name;
            size = other.size;
            delete pointsTo;
            pointsTo = nullptr;
            if (other.pointsTo != nullptr) {
                pointsTo = new DataType(*other.pointsTo);
            }
        }
        return *this;
    }

    DataType FinalPointsTo() const {
        if (pointsTo == nullptr) {
            return *this;
        }
        return pointsTo->FinalPointsTo();
    }

    std::string ToString() const {
        if (pointsTo == nullptr) {
            return name;
        }
        return name + " -> " + pointsTo->ToString();
    }
};

struct FrameParam {
    std::string name;
    DataType type;
};

class Node {
   public:
    NodeType type;

    // CONST INT
    int value;

    // FRAME
    std::string functionName;
    std::vector<std::unique_ptr<ast::Node>> body;
    std::vector<FrameParam> params;

    std::string variableName = "";
    std::string tempName = "";

    // BIN
    std::unique_ptr<Node> lhs;
    std::unique_ptr<Node> rhs;
    BinOpKind binOpKind;

    // UNARY
    std::unique_ptr<Node> expr;

    // jump
    std::string jumpToLabelValue;

    // variable
    ast::DataType variableType;

    // if
    std::unique_ptr<Node> condition;
    std::vector<std::unique_ptr<ast::Node>> then;
    std::vector<std::unique_ptr<ast::Node>> else_;

    // call
    std::string callName;
    std::vector<std::unique_ptr<ast::Node>> callArgs;
    ast::DataType returnType;

    // for
    std::unique_ptr<Node> forInit;
    std::optional<std::unique_ptr<Node>> forCondition;
    std::optional<std::unique_ptr<Node>> forUpdate;
    std::vector<std::unique_ptr<ast::Node>> forBody;

    int derefDepth = 0;

    std::string VariableName() {
        if (type == NodeType::Var) {
            return variableName;
        }
        if (type == NodeType::Deref) {
            return expr->VariableName();
        }
        throw std::runtime_error("VariableName not implemented for type: " +
                                 std::to_string(static_cast<int>(type)));
    }
};

std::unique_ptr<Node> makeConstInt(int value);

std::unique_ptr<Node> makeNewFunction(
    std::string functionName, std::vector<std::unique_ptr<ast::Node>> body,
    std::vector<FrameParam> params);

std::unique_ptr<Node> makeNewReturn(std::unique_ptr<ast::Node> expr);
std::unique_ptr<Node> makeNewVar(std::string name, DataType type);

std::unique_ptr<Node> makeNewMove(std::unique_ptr<ast::Node> left,
                                  std::unique_ptr<ast::Node> right);

std::unique_ptr<Node> makeNewMemWrite(std::unique_ptr<ast::Node> expr);
std::unique_ptr<Node> makeNewMemRead(std::unique_ptr<ast::Node> expr);
std::unique_ptr<Node> makeNewAddr(std::unique_ptr<ast::Node> expr);
std::unique_ptr<Node> makeNewBinOp(std::unique_ptr<ast::Node> lhs,
                                   std::unique_ptr<ast::Node> rhs,
                                   BinOpKind kind);
std::unique_ptr<Node> makeNewIfStmt(
    std::unique_ptr<ast::Node> condition,
    std::vector<std::unique_ptr<ast::Node>> then,
    std::vector<std::unique_ptr<ast::Node>> else_);

std::unique_ptr<Node> makeNewCall(std::string name,
                                  std::vector<std::unique_ptr<ast::Node>> args);
std::unique_ptr<Node> makeNewForLoop(
    std::unique_ptr<ast::Node> init,
    std::optional<std::unique_ptr<ast::Node>> condition,
    std::optional<std::unique_ptr<ast::Node>> update,
    std::vector<std::unique_ptr<ast::Node>> body);

inline std::ostream& operator<<(std::ostream& os, const Node& node);
inline std::ostream& DebugFrame(std::ostream& os, const Node& node) {
    os << "Frame(name=" << node.functionName << ", params=[";
    for (const auto& param : node.params) {
        os << param.name << ", ";
    }
    os << "], body=[";
    for (const auto& n : node.body) {
        os << *n << std::endl;
    }
    os << "])";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    switch (node.type) {  //
        case NodeType::Move:
            os << "Move(" << *node.lhs << ", " << *node.rhs << ")";
            break;
        case NodeType::Return:
            os << "Return(" << *node.expr << ")";
            break;
        case NodeType::Temp:
            os << "Temp(" << node.tempName << ")";
            break;
        case NodeType::Seq:
            os << "Seq(";
            for (const auto& n : node.body) {
                os << *n << ", ";
            }
            os << ")";
            break;
        case NodeType::Jump:
            os << "Jump(" << node.jumpToLabelValue << ")";
            break;
        case NodeType::Var:
            os << "Var(variableName=" << node.variableName << ")";
            break;
        case NodeType::Deref:
            os << "Deref(" << *node.expr << ")";
            break;
        case NodeType::Addr:
            os << "Addr(" << *node.expr << ")";
            break;
        case NodeType::ConstInt:
            os << "ConstInt(" << node.value << ")";
            break;
        case NodeType::Frame:
            DebugFrame(os, node);
            break;
        case NodeType::BinOp:
            os << "BinOp(" << *node.lhs << ", " << *node.rhs << ")";
            break;
        case NodeType::If:
            os << "If(" << *node.condition << ", then=[";
            for (const auto& n : node.then) {
                os << *n << ", ";
            }
            os << "], else=[";
            for (const auto& n : node.else_) {
                os << *n << ", ";
            }
            os << "])";
            break;
        case NodeType::Call:
            os << "Call(" << node.callName << ", args=[";
            for (const auto& n : node.callArgs) {
                os << *n << ", ";
            }
            os << "])";
            break;
        case NodeType::ForLoop: {
            os << "ForLoop(init=" << *node.forInit
               << ", condition=" << **node.forCondition
               << ", update=" << **node.forUpdate << ", body=[";
            for (const auto& n : node.forBody) {
                os << *n << ", ";
            }
            os << "])";
            break;
        }
        default: {
            os << "Unknown(" << static_cast<int>(node.type) << ")";
            break;
        }
    }
    return os;
}
}  // namespace ast
