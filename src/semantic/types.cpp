#include "semantic/types.h"

#include <utility>

namespace compilerlab::semantic {

Type::Type(TypeKind kind)
    : kind(kind) {
}

BuiltinType::BuiltinType(TypeKind kind)
    : Type(kind) {
}

FunctionType::FunctionType(TypePtr return_type, std::vector<TypePtr> parameter_types)
    : Type(TypeKind::Function),
      return_type(std::move(return_type)),
      parameter_types(std::move(parameter_types)) {
}

TypeKind type_kind_from_specifier(TypeSpecifier specifier) {
    switch (specifier) {
        case TypeSpecifier::Int: return TypeKind::Int;
        case TypeSpecifier::Float: return TypeKind::Float;
        case TypeSpecifier::Void: return TypeKind::Void;
    }
    return TypeKind::Error;
}

TypePtr make_builtin_type(TypeKind kind) {
    return std::make_shared<BuiltinType>(kind);
}

std::string to_string(TypeKind kind) {
    switch (kind) {
        case TypeKind::Int: return "int";
        case TypeKind::Float: return "float";
        case TypeKind::Void: return "void";
        case TypeKind::Function: return "function";
        case TypeKind::Error: return "error";
    }
    return "unknown-type";
}

}  // namespace compilerlab::semantic
