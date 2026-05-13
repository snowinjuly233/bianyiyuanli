#include "runtime/scanner_runtime.h"

#include <optional>
#include <utility>

namespace compilerlab::runtime {

namespace {

common::SourceLocation locate(std::string_view input, std::size_t offset) {
    common::SourceLocation location;
    location.offset = offset;
    location.line = 1;
    location.column = 1;

    std::size_t start = 0;
    if (input.size() >= 3 &&
        static_cast<unsigned char>(input[0]) == 0xEF &&
        static_cast<unsigned char>(input[1]) == 0xBB &&
        static_cast<unsigned char>(input[2]) == 0xBF) {
        start = std::min<std::size_t>(3, offset);
    }

    for (std::size_t index = start; index < offset && index < input.size(); ++index) {
        if (input[index] == '\n') {
            ++location.line;
            location.column = 1;
        } else {
            ++location.column;
        }
    }
    return location;
}

}  // namespace

ScannerRuntime::ScannerRuntime(std::vector<ScannerState> states)
    : states_(std::move(states)) {
}

common::Token ScannerRuntime::scan_one(std::string_view input, std::size_t& offset,
                                       const std::string& file_path) const {
    while (true) {
        if (offset == 0 && input.size() >= 3 &&
            static_cast<unsigned char>(input[0]) == 0xEF &&
            static_cast<unsigned char>(input[1]) == 0xBB &&
            static_cast<unsigned char>(input[2]) == 0xBF) {
            offset = 3;
        }

        common::Token token;
        token.span.file_path = file_path;
        token.span.begin = locate(input, offset);
        token.span.end = token.span.begin;

        if (offset >= input.size()) {
            token.kind = common::TokenKind::EndOfFile;
            return token;
        }

        if (states_.empty()) {
            token.kind = common::TokenKind::Invalid;
            token.lexeme.assign(1, input[offset]);
            ++offset;
            token.span.end = locate(input, offset);
            return token;
        }

        struct AcceptSnapshot {
            ScannerAcceptAction action;
            std::size_t end_offset {0};
        };

        int current_state = 0;
        std::size_t cursor = offset;
        std::optional<AcceptSnapshot> last_accept;

        if (states_[0].accept.has_value()) {
            last_accept = AcceptSnapshot{*states_[0].accept, offset};
        }

        while (cursor < input.size()) {
            const auto transition = states_[static_cast<std::size_t>(current_state)].transitions.find(input[cursor]);
            if (transition == states_[static_cast<std::size_t>(current_state)].transitions.end()) {
                break;
            }

            current_state = transition->second;
            ++cursor;

            const auto& state = states_[static_cast<std::size_t>(current_state)];
            if (state.accept.has_value()) {
                last_accept = AcceptSnapshot{*state.accept, cursor};
            }
        }

        if (!last_accept.has_value()) {
            token.kind = common::TokenKind::Invalid;
            token.lexeme.assign(1, input[offset]);
            ++offset;
            token.span.end = locate(input, offset);
            return token;
        }

        token.kind = last_accept->action.kind;
        token.lexeme = std::string(input.substr(offset, last_accept->end_offset - offset));
        offset = last_accept->end_offset;
        token.span.end = locate(input, offset);

        if (last_accept->action.skip) {
            continue;
        }

        return token;
    }
}

}  // namespace compilerlab::runtime
