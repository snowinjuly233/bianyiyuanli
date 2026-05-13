#pragma once

#include <memory>
#include <string>
#include <vector>

#include "semantic/ast.h"

namespace compilerlab::semantic {

enum class TypeKind {
    Int,
    Float,
    Void,
    Function,
    Error,
};

struct Type {
    explicit Type(TypeKind kind);
    virtual ~Type() = default;

    TypeKind kind;
};

using TypePtr = std::shared_ptr<Type>;

struct BuiltinType final : Type {
    explicit BuiltinType(TypeKind kind);
};

struct FunctionType final : Type {
    FunctionType(TypePtr return_type, std::vector<TypePtr> parameter_types);

    TypePtr return_type;
    std::vector<TypePtr> parameter_types;
};

TypeKind type_kind_from_specifier(TypeSpecifier specifier);
TypePtr make_builtin_type(TypeKind kind);
std::string to_string(TypeKind kind);

}  // namespace compilerlab::semantic
