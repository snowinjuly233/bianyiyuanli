#include "ir/quad.h"

namespace compilerlab::ir {

std::string to_string(OpCode op) {
    switch (op) {
        case OpCode::Add: return "add";
        case OpCode::Subtract: return "sub";
        case OpCode::Multiply: return "mul";
        case OpCode::Divide: return "div";
        case OpCode::Modulo: return "mod";
        case OpCode::Assign: return "assign";
        case OpCode::CompareEqual: return "cmp_eq";
        case OpCode::CompareNotEqual: return "cmp_ne";
        case OpCode::CompareLess: return "cmp_lt";
        case OpCode::CompareLessEqual: return "cmp_le";
        case OpCode::CompareGreater: return "cmp_gt";
        case OpCode::CompareGreaterEqual: return "cmp_ge";
        case OpCode::LogicalAnd: return "and";
        case OpCode::LogicalOr: return "or";
        case OpCode::LogicalNot: return "not";
        case OpCode::Jump: return "jmp";
        case OpCode::JumpIfFalse: return "jmp_false";
        case OpCode::Param: return "param";
        case OpCode::Call: return "call";
        case OpCode::Return: return "ret";
        case OpCode::Label: return "label";
    }
    return "unknown-op";
}

}  // namespace compilerlab::ir
