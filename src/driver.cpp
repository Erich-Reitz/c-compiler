#include <fstream>
#include <iostream>
#include <sstream>

#include "../include/compiler/qa_ir/assem.hpp"
#include "../include/compiler/target/allocator.hpp"
#include "../include/compiler/target/codegen.hpp"
#include "../include/compiler/target/lower_ir.hpp"
#include "../include/compiler/translate.hpp"
#include "../include/lexer/lexer.hpp"
#include "../include/parser/parser.hpp"

#define DEBUG 1

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
        std::cout << "Function: " << frame.name << std::endl;
        for (const auto& ins : frame.instructions) {
            std::cout << ins << std::endl;
        }
        std::cout << "-----------------" << std::endl;
    }
    std::cout << "-----------------" << std::endl;
}

void print_target_ir(const std::vector<target::Frame>& frames) {
    std::cout << "-----------------" << std::endl;
    std::cout << "TARGET IR:" << std::endl;
    for (const auto& frame : frames) {
        std::cout << "Function: " << frame.name << std::endl;
        for (const auto& ins : frame.instructions) {
            std::cout << std::visit([](const auto& arg) { return arg.debug_str(); }, ins)
                      << std::endl;
        }
        std::cout << "-----------------" << std::endl;
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
    if (DEBUG) print_target_ir(lowered_frames);

    const auto rewritten = target::rewrite(lowered_frames);

    const auto code = target::Generate(rewritten);
    write_to_file(code, outfile);

    return 0;
}
