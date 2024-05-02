#pragma once

#include <string>

namespace target {

class CodegenContext {
   public:
    std::string DataSection = "section .data\n";
    std::string Code = "";

    void AddInstructionNoIndent(const std::string& i); 

    void AddInstruction(const std::string& i); 

    std::string DefineFloatConstant(float value);
  private:
    int float_counter = 0;
};
}