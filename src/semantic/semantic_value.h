#pragma once

#include <cstdint>
#include <type_traits>
#include <string>
#include <utility>
#include <variant>

#include "common/token.h"
#include "semantic/ast.h"

namespace compilerlab::semantic {

using SemanticValueData = std::variant<
    std::monostate,
    std::int64_t,
    double,
    std::string,
    compilerlab::common::Token,
    TypeSpecifier,
    ProgramPtr,
    DeclPtr,
    ParameterDeclPtr,
    StmtPtr,
    ExprPtr,
    DeclList,
    ParameterList,
    StmtList,
    ExprList>;

class SemanticValue {
public:
    SemanticValue() = default;
    SemanticValue(const SemanticValue&) = default;
    SemanticValue(SemanticValue&&) noexcept = default;
    SemanticValue& operator=(const SemanticValue&) = default;
    SemanticValue& operator=(SemanticValue&&) noexcept = default;

    template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, SemanticValue>>>
    SemanticValue(T&& value)
        : data_(std::forward<T>(value)) {
    }

    template <typename T>
    bool is() const {
        return std::holds_alternative<T>(data_);
    }

    template <typename T>
    T& as() {
        return std::get<T>(data_);
    }

    template <typename T>
    const T& as() const {
        return std::get<T>(data_);
    }

    template <typename T>
    const T* get_if() const {
        return std::get_if<T>(&data_);
    }

    bool empty() const {
        return std::holds_alternative<std::monostate>(data_);
    }

    const SemanticValueData& data() const {
        return data_;
    }

private:
    SemanticValueData data_ {};
};

}  // namespace compilerlab::semantic
