#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "common/token.h"

namespace compilerlab::runtime {

struct ScannerAcceptAction {
    common::TokenKind kind {common::TokenKind::Invalid};
    std::size_t rule_priority {0};
    bool skip {false};
};

struct ScannerState {
    int id {0};
    std::optional<ScannerAcceptAction> accept;
    std::unordered_map<char, int> transitions;
};

class ScannerRuntime {
public:
    explicit ScannerRuntime(std::vector<ScannerState> states = {});

    common::Token scan_one(std::string_view input, std::size_t& offset,
                           const std::string& file_path = {}) const;

private:
    std::vector<ScannerState> states_;
};

}  // namespace compilerlab::runtime
