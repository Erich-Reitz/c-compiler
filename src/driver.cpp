#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../include/compiler/allocator.hpp"
#include "../include/compiler/assem.hpp"
#include "../include/compiler/codegen.hpp"
#include "../include/compiler/lower_ir.hpp"
#include "../include/compiler/translate.hpp"
#include "../include/lexer/lexer.hpp"
#include "../include/parser/parser.hpp"
#include "../include/parser/st.hpp"

#define DEBUG 0

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

void print_ast(const std::vector<ast::TopLevelNode>& ast) {
    std::cout << "-----------------" << std::endl;
    std::cout << "AST:" << std::endl;
    for (const auto& node : ast) {
        std::cout << node.toString() << std::endl;
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

void print_lower_ir(const std::vector<target::Frame>& frames, const std::string& header) {
    std::cout << "-----------------" << std::endl;
    std::cout << header << std::endl;
    for (const auto& frame : frames) {
        for (const auto& ins : frame.instructions) {
            // std::cout << ins << std::endl;
        }
    }
    std::cout << "-----------------" << std::endl;
}

void write_to_file(const std::string& code, const std::string& outfile) {
    std::ofstream outFile;
    outFile.open(outfile);
    outFile << code;
    outFile.close();
}

int runfile(const char* sourcefile, const std::string& outfile) {
    const auto contents = readfile(sourcefile);
    const auto tokens = lexer::lex(contents);
    const auto st = parse(tokens);

    if (DEBUG) print_syntax_tree(st);

    auto ast = ast::translate(st);

    if (DEBUG) print_ast(ast);

    auto frames = qa_ir::Produce_IR(ast);

    if (DEBUG) print_ir(frames);

    const auto lowered_frames = target::LowerIR(frames);

    if (DEBUG) print_lower_ir(lowered_frames, "Lowered IR:");

    auto rewritten = target::rewrite(lowered_frames);

    if (DEBUG) print_lower_ir(rewritten, "Rewritten IR:");

    auto code = codegen::Generate(rewritten);
    write_to_file(code, outfile);

    return 0;
}
