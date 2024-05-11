#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "../parser/st.hpp"
#include "ast.hpp"

namespace ast {

template <typename T>
concept ContainsTypeDeclaration = requires(T t) {
    { t.declarationSpecifiers } -> std::convertible_to<std::vector<st::DeclarationSpecifier>>;
    { t.GetDeclarator() } -> std::convertible_to<std::optional<st::Declarator>>;
};

// TODO: use the dss parameter here
[[nodiscard]] DataType toDataType(const std::vector<st::DeclarationSpecifier>& dss) {
    for (const auto& ds : dss) {
        if (ds.typespecifier.type == st::TypeSpecifier::Type::INT) {
            return DataType::int_type();
        } else if (ds.typespecifier.type == st::TypeSpecifier::Type::FLOAT) {
            return DataType::float_type();
        }
    }
    throw std::runtime_error("Unsupported type specifier");
}

[[nodiscard]] DataType toDataType(const st::Declarator& decl, DataType pointsTo) {
    const auto directDeclarator = decl.directDeclarator;
    if (directDeclarator.kind == st::DeclaratorKind::VARIABLE) {
        return DataType{
            .base_type = BaseType::POINTER, .points_to = pointsTo.base_type, .indirect_level = 1};
    } else if (directDeclarator.kind == st::DeclaratorKind::ARRAY) {
        const auto info = std::get<st::ArrayDirectDeclarator>(directDeclarator.declarator);
        const auto expression = std::get<std::shared_ptr<st::PrimaryExpression>>(info.size);
        return DataType::array_type(expression->value, pointsTo.base_type);
    } else {
        throw std::runtime_error("Unsupported declarator kind");
    }
}

template <ContainsTypeDeclaration T>
DataType toDataType(const T& decl) {
    auto datatype = toDataType(decl.declarationSpecifiers);
    std::optional<st::Declarator> opt_declarator = decl.GetDeclarator();
    if (!opt_declarator) {
        return datatype;
    }
    auto declarator = opt_declarator.value();

    if (declarator.pointer || declarator.directDeclarator.kind == st::DeclaratorKind::ARRAY) {
        datatype = toDataType(declarator, datatype);
    }
    return datatype;
}
}  // namespace ast
