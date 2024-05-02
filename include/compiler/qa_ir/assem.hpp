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

#include "qa_ir.hpp"
#include "qa_ir_operations.hpp"
#include "../../ast/ast.hpp"

namespace qa_ir {

struct Frame {
    std::string name;
    std::vector<Operation> instructions;
    int size = 0;
};

struct F_Ctx {
    int temp_counter = 0;
    int label_counter = 0;
    std::map<std::string, Variable> variables = {};

    [[nodiscard]] Value AddVariable(std::string name, ast::DataType type) {
        variables[name] = Variable{.name = name, .type = type, .offset = nullptr};
        return variables[name];
    }

    [[nodiscard]] Temp AddTemp(int size) { return Temp{.id = temp_counter++, .size = size}; }

    [[nodiscard]] Label AddLabel() {
        auto label = "L" + std::to_string(label_counter);
        label_counter++;
        return Label{label};
    }
};

[[nodiscard]] auto Produce_IR(std::vector<ast::TopLevelNode>& nodes) -> std::vector<Frame>;

}  // namespace qa_ir