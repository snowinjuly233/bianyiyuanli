#include "ir/basic_block.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace compilerlab::ir {

namespace {

bool is_label(const Quad& quad) {
    return quad.op == OpCode::Label;
}

bool is_unconditional_jump(const Quad& quad) {
    return quad.op == OpCode::Jump;
}

bool is_conditional_jump(const Quad& quad) {
    return quad.op == OpCode::JumpIfFalse;
}

bool is_return(const Quad& quad) {
    return quad.op == OpCode::Return;
}

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

std::string format_quad(const Quad& quad) {
    std::ostringstream output;

    if (quad.op == OpCode::Label) {
        output << operand_text(quad.result) << ":";
        return output.str();
    }

    output << to_string(quad.op);

    bool printed_result = false;
    if (quad.result.kind != Operand::Kind::Empty) {
        output << " " << operand_text(quad.result);
        printed_result = true;
    }

    const bool has_args = quad.arg1.kind != Operand::Kind::Empty ||
                          quad.arg2.kind != Operand::Kind::Empty;
    if (has_args) {
        output << (printed_result ? " = " : " ");
        bool first = true;
        if (quad.arg1.kind != Operand::Kind::Empty) {
            output << operand_text(quad.arg1);
            first = false;
        }
        if (quad.arg2.kind != Operand::Kind::Empty) {
            if (!first) {
                output << ", ";
            }
            output << operand_text(quad.arg2);
        }
    }

    return output.str();
}

}  // namespace

std::vector<BasicBlock> build_basic_blocks(const std::vector<Quad>& quads) {
    std::vector<BasicBlock> blocks;
    if (quads.empty()) {
        return blocks;
    }

    std::vector<std::size_t> leaders {0};
    for (std::size_t index = 0; index < quads.size(); ++index) {
        const auto& quad = quads[index];
        if (index > 0 && is_label(quad)) {
            leaders.push_back(index);
        }
        if ((is_unconditional_jump(quad) || is_conditional_jump(quad) || is_return(quad)) &&
            index + 1 < quads.size()) {
            leaders.push_back(index + 1);
        }
    }

    std::sort(leaders.begin(), leaders.end());
    leaders.erase(std::unique(leaders.begin(), leaders.end()), leaders.end());

    std::unordered_map<std::string, std::size_t> block_index_by_name;
    for (std::size_t leader_index = 0; leader_index < leaders.size(); ++leader_index) {
        const auto begin = leaders[leader_index];
        const auto end = leader_index + 1 < leaders.size() ? leaders[leader_index + 1] : quads.size();

        BasicBlock block;
        if (is_label(quads[begin]) && !quads[begin].result.text.empty()) {
            block.name = quads[begin].result.text;
        } else {
            block.name = "bb" + std::to_string(leader_index);
        }

        block.quads.insert(block.quads.end(), quads.begin() + static_cast<std::ptrdiff_t>(begin),
                           quads.begin() + static_cast<std::ptrdiff_t>(end));
        block_index_by_name[block.name] = blocks.size();
        blocks.push_back(std::move(block));
    }

    for (std::size_t index = 0; index < blocks.size(); ++index) {
        auto& block = blocks[index];
        if (block.quads.empty()) {
            continue;
        }

        const auto& terminator = block.quads.back();
        if (is_unconditional_jump(terminator)) {
            if (!terminator.result.text.empty()) {
                block.successors.push_back(terminator.result.text);
            }
            continue;
        }

        if (is_conditional_jump(terminator)) {
            if (!terminator.result.text.empty()) {
                block.successors.push_back(terminator.result.text);
            }
            if (index + 1 < blocks.size()) {
                block.successors.push_back(blocks[index + 1].name);
            }
            continue;
        }

        if (is_return(terminator)) {
            continue;
        }

        if (index + 1 < blocks.size()) {
            block.successors.push_back(blocks[index + 1].name);
        }
    }

    return blocks;
}

std::string BasicBlockPrinter::print(const std::vector<BasicBlock>& blocks) const {
    std::ostringstream output;
    output << "# Basic Blocks\n\n";

    for (const auto& block : blocks) {
        output << "## " << block.name << "\n\n";
        output << "Successors: ";
        if (block.successors.empty()) {
            output << "(none)";
        } else {
            for (std::size_t index = 0; index < block.successors.size(); ++index) {
                if (index > 0) {
                    output << ", ";
                }
                output << block.successors[index];
            }
        }
        output << "\n\n";
        output << "```text\n";
        for (const auto& quad : block.quads) {
            output << format_quad(quad) << "\n";
        }
        output << "```\n\n";
    }

    return output.str();
}

}  // namespace compilerlab::ir
