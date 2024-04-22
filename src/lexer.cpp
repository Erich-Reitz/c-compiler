#include "../include/lexer.hpp"

#include <ctype.h>

#include <cassert>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/token.hpp"
namespace lexer {

static unsigned long current = 0;
static unsigned long start = 0;
static unsigned long line = 1;
std::string source;

const std::unordered_map<std::string, TokType> keywords = {
    {"return", TokType::TOKEN_RETURN}, {"int", TokType::TOKEN_T_INT},
    {"else", TokType::TOKEN_ELSE},     {"if", TokType::TOKEN_IF},
    {"for", TokType::TOKEN_FOR},
};

auto isAtEnd() -> bool { return current >= source.size(); }

auto advance() -> char {
    assert(!isAtEnd());
    return source[current++];
}

auto peek() -> char {
    if (isAtEnd()) return '\0';
    return source[current];
}

[[nodiscard]] char peekNext() {
    if (current + 1 >= source.size()) return '\0';
    return source[current + 1];
}

[[nodiscard]] std::optional<Token> scanToken() {
    char c = advance();
    switch (c) {
        case '&':
            return Token{TokType::TOKEN_AMPERSAND, "&"};
        case '(':
            return Token{TokType::TOKEN_LEFT_PAREN, "("};
        case ')':
            return Token{TokType::TOKEN_RIGHT_PAREN, ")"};
        case '{':
            return Token{TokType::TOKEN_LEFT_BRACE, "{"};
        case '}':
            return Token{TokType::TOKEN_RIGHT_BRACE, "}"};
        case ',':
            return Token{TokType::TOKEN_COMMA, ","};
        case '.':
            return Token{TokType::TOKEN_DOT, "."};
        case '-':
            return Token{TokType::TOKEN_MINUS, "-"};
        case '+':
            return Token{TokType::TOKEN_PLUS, "+"};
        case ';':
            return Token{TokType::TOKEN_SEMICOLON, ";"};
        case '*':
            return Token{TokType::TOKEN_STAR, "*"};
        case '!':
            if (peek() == '=') {
                advance();
                return Token{TokType::TOKEN_BANG_EQUAL, "!="};
            }
            return Token{TokType::TOKEN_BANG, "!"};
        case '=':
            if (peek() == '=') {
                advance();
                return Token{TokType::TOKEN_EQUAL_EQUAL, "=="};
            }
            return Token{TokType::TOKEN_EQUAL, "="};
        case '<':
            if (peek() == '=') {
                advance();
                return Token{TokType::TOKEN_LESS_EQUAL, "<="};
            }
            return Token{TokType::TOKEN_LESS, "<"};
        case '>':
            if (peek() == '=') {
                advance();
                return Token{TokType::TOKEN_GREATER_EQUAL, ">="};
            }
            return Token{TokType::TOKEN_GREATER, ">"};
        case '/':
            if (peek() == '/') {
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
            } else {
                return Token{TokType::TOKEN_SLASH, "/"};
            }
        case ' ':
        case '\r':
        case '\t':
        case '\0':
            break;
        case '\n':
            line++;
            break;
        default:
            if (isdigit(c)) {
                while (isdigit(peek())) advance();
                if (peek() == '.' && isdigit(peekNext())) {
                    advance();
                    while (isdigit(peek())) advance();
                }
                return Token{TokType::TOKEN_NUMBER,
                             source.substr(start, current - start)};
            } else if (isalpha(c)) {
                while (isalnum(peek()) || peek() == '_') {
                    advance();
                }
                assert(current - 1 < source.size());
                const std::string text = source.substr(start, current - start);
                if (keywords.find(text) != keywords.end()) {
                    return Token{keywords.at(text), text};
                }
                return Token{TokType::TOKEN_IDENTIFIER, text};
            } else {
                fprintf(stderr, "Unexpected character '%d' on line %zu\n", c,
                        line);
                exit(EXIT_FAILURE);
            }
    }
    return std::nullopt;
}

[[nodiscard]] std::vector<Token> lex(const std::string& src) {
    source = src;
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        const auto tk = scanToken();
        if (tk.has_value()) tokens.push_back(tk.value());
        start = current;
    }
    tokens.push_back(Token{TokType::TOKEN_FEOF, ""});
    return tokens;
}
}  // namespace lexer