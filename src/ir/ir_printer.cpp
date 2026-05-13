#include "ir/ir_printer.h"

#include <sstream>

namespace compilerlab::ir {

std::string operand_text(const Operand& op) {
    switch (op.kind) {
        case Operand::Kind::Empty: return "";
        case Operand::Kind::Temporary: return op.text;
        case Operand::Kind::Identifier: return op.text;
        case Operand::Kind::IntegerLiteral: return op.text;
        case Operand::Kind::FloatLiteral: return op.text;
        case Operand::Kind::Label: return op.text;
    }
    return "";
}

std::string IRPrinter::print(const std::vector<Quad>& quads) const {
    std::ostringstream output;
    for (const auto& quad : quads) {
        if (quad.op == OpCode::Label) {
            output << operand_text(quad.result) << ":\n";
            continue;
        }

        output << "    " << to_string(quad.op) << " ";

        bool printed_result = false;
        if (quad.result.kind != Operand::Kind::Empty) {
            output << operand_text(quad.result);
            printed_result = true;
        }

        bool has_args = quad.arg1.kind != Operand::Kind::Empty || quad.arg2.kind != Operand::Kind::Empty;
        if (has_args) {
            if (printed_result) output << " = ";
            bool first = true;
            if (quad.arg1.kind != Operand::Kind::Empty) {
                output << operand_text(quad.arg1);
                first = false;
            }
            if (quad.arg2.kind != Operand::Kind::Empty) {
                if (!first) output << ", ";
                output << operand_text(quad.arg2);
            }
        }

        output << "\n";
    }
    return output.str();
}

}  // namespace compilerlab::ir