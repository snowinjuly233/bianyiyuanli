#pragma once

#include <string>
#include <vector>

namespace compilerlab::ir {

enum class OpCode {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Assign,
    CompareEqual,
    CompareNotEqual,
    CompareLess,
    CompareLessEqual,
    CompareGreater,
    CompareGreaterEqual,
    LogicalAnd,
    LogicalOr,
    LogicalNot,
    Jump,
    JumpIfFalse,
    Param,
    Call,
    Return,
    Label,
};

struct Operand {
    enum class Kind {
        Empty,
        Temporary,
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        Label,
    };

    Kind kind {Kind::Empty};
    std::string text;
};

struct Quad {
    OpCode op {OpCode::Assign};
    Operand arg1 {};
    Operand arg2 {};
    Operand result {};
};

std::string to_string(OpCode op);

}  // namespace compilerlab::ir
