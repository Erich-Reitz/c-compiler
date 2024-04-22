#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../include/allocator.hpp"
#include "../include/assem.hpp"
#include "../include/codegen.hpp"
#include "../include/lexer.hpp"
#include "../include/lower_ir.hpp"
#include "../include/parser.hpp"
#include "../include/st.hpp"
#include "../include/translate.hpp"

[[nodiscard]] const std::string readfile(const char* sourcefile) {
    std::ifstream inFile;
    inFile.open(sourcefile);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    return strStream.str();
}

void print_syntax_tree(const st::Program& st) {
    std::cout << "-----------------" << std::endl;
    std::cout << "Syntax Tree:" << std::endl;
    for (const auto& node : st.nodes) {
        std::cout << node << std::endl;
    }
    std::cout << "-----------------" << std::endl;
}

void print_ast(const std::vector<std::unique_ptr<ast::Node>>& ast) {
    std::cout << "-----------------" << std::endl;
    std::cout << "AST:" << std::endl;
    for (const auto& node : ast) {
        std::cout << *node << std::endl;
    }
    std::cout << "-----------------" << std::endl;
}

void print_ir(const std::vector<qa_ir::Frame>& frames) {
    std::cout << "-----------------" << std::endl;
    std::cout << "IR:" << std::endl;
    for (const auto& frame : frames) {
        for (const auto& ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
}

void print_lower_ir(const std::vector<target::Frame>& frames) {
    for (const auto& frame : frames) {
        for (const auto& ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
}

void write_to_file(const std::string& code) {
    std::ofstream outFile;
    outFile.open("test.asm");
    outFile << code;
    outFile.close();
}

int runfile(const char* sourcefile) {
    const auto contents = readfile(sourcefile);
    const auto tokens = lexer::lex(contents);
    const auto st = parse(tokens);
    print_syntax_tree(st);
    const auto ast = translate(st);
    print_ast(ast);
    auto frames = qa_ir::Produce_IR(ast);
    print_ir(frames);
    const auto lowered_frames = target::LowerIR(frames);
    std::cout << "-----------------" << std::endl;
    std::cout << "Lowered IR:" << std::endl;
    print_lower_ir(lowered_frames);
    auto rewritten = target::rewrite(lowered_frames);
    std::cout << "-----------------" << std::endl;
    std::cout << "Rewritten IR:" << std::endl;
    print_lower_ir(rewritten);
    auto code = codegen::Generate(rewritten);
    write_to_file(code);
    return 0;
}
