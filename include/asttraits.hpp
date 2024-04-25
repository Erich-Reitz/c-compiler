#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <string>

#include "ast.hpp"
#include "st.hpp"

namespace asttraits {

template <typename T>
concept ContainsTypeDeclaration = requires(T t) {
    { t.declarationSpecifiers } -> std::convertible_to<std::vector<st::DeclarationSpecifier>>;
    { t.GetDeclarator() } -> std::convertible_to<std::optional<st::Declarator>>;
};

[[nodiscard]] ast::DataType* toDataType(const std::vector<st::DeclarationSpecifier>& dss) {
    return new ast::DataType("int", 4, nullptr);
}

[[nodiscard]] ast::DataType* toDataType(const st::Declarator& decl, ast::DataType* pointsTo) {
    auto ptr = decl.pointer;
    if (!ptr) return nullptr;
    auto levels = ptr.value().level;
    ast::DataType* result = pointsTo;
    for (size_t i = 0; i < levels; i++) {
        result = new ast::DataType("pointer", 8, result);
    }
    return result;
}

template <ContainsTypeDeclaration T>
ast::DataType* toDataType(const T& decl) {
    auto datatype = toDataType(decl.declarationSpecifiers);
    std::optional<st::Declarator> opt_declarator = decl.GetDeclarator();
    if (!opt_declarator) {
        return datatype;
    }
    auto declarator = opt_declarator.value();
    if (declarator.pointer) {
        datatype = toDataType(declarator, datatype);
    }
    return datatype;
}
}  // namespace asttraits
