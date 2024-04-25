#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "ast.hpp"
#include "st.hpp"

namespace asttraits {

template <typename T>
concept ContainsTypeDeclaration = requires(T t) {
    { t.declarationSpecifiers } -> std::convertible_to<std::vector<st::DeclarationSpecifier>>;
    { t.GetDeclarator() } -> std::convertible_to<std::optional<st::Declarator>>;
};

[[nodiscard]] ast::DataType toDataType(const std::vector<st::DeclarationSpecifier>& dss) {
    return ast::DataType{.name = "int", .size = 4, .is_pointer = false, .points_to_size = 0};
}

[[nodiscard]] ast::DataType toDataType(const st::Declarator& decl, ast::DataType pointsTo) {
    return ast::DataType{
        .name = pointsTo.name, .size = 8, .is_pointer = true, .points_to_size = pointsTo.size};
}

template <ContainsTypeDeclaration T>
ast::DataType toDataType(const T& decl) {
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
