#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "qa_ir.hpp"
#include "qa_x86.hpp"

namespace qa_ir {

struct Frame {
    std::string name;
    std::vector<Operation> instructions;
    int size = 0;
};

struct Ctx {
    int counter;
    int labelCounter;
    std::map<std::string, int> variableUsage;
    std::map<std::string, const ast::Node*> variables;

    Temp newTemp(int size) {
        assert(size != 0);
        return Temp{counter++, size};
    }

    Value AddVariable(const ast::Node* node) {
        auto name = node->variableName;
        variableUsage[name]++;
        variables[name] = node;
        const auto size = node->variableType.size;
        return Variable{name, variableUsage[name], size};
    }

    Value getVariable(const std::string& name) {
        return Variable{name, variableUsage[name],
                        variables[name]->variableType.size};
    }

    Label newLabel() {
        auto label = "L" + std::to_string(labelCounter);
        labelCounter++;
        return Label{label};
    }
};

CondJ GenerateConditionalIR(std::vector<Operation>& ins,
                            const std::unique_ptr<ast::Node>& condition,
                            Ctx& ctx);

[[nodiscard]] std::vector<Frame> Produce_IR(
    const std::vector<std::unique_ptr<ast::Node>>& nodes);

}  // namespace qa_ir