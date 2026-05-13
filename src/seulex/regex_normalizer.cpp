#include "seulex/regex_normalizer.h"

#include <cctype>
#include <string_view>
#include <unordered_set>

namespace compilerlab::seulex {

namespace {

bool is_definition_name(std::string_view text) {
    if (text.empty()) {
        return false;
    }

    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch)) == 0 && ch != '_') {
            return false;
        }
    }
    return true;
}

std::string normalize_impl(const std::string& pattern,
                           const std::unordered_map<std::string, std::string>& definitions,
                           std::unordered_set<std::string>& active,
                           common::DiagnosticEngine& diagnostics) {
    std::string result;
    bool in_quote = false;
    bool in_class = false;
    bool escaped = false;

    for (std::size_t index = 0; index < pattern.size(); ++index) {
        const char ch = pattern[index];
        if (escaped) {
            result.push_back(ch);
            escaped = false;
            continue;
        }

        if (ch == '\\') {
            result.push_back(ch);
            escaped = true;
            continue;
        }

        if (ch == '"' && !in_class) {
            result.push_back(ch);
            in_quote = !in_quote;
            continue;
        }

        if (!in_quote) {
            if (ch == '[') {
                result.push_back(ch);
                in_class = true;
                continue;
            }

            if (ch == ']' && in_class) {
                result.push_back(ch);
                in_class = false;
                continue;
            }

            if (ch == '{' && !in_class) {
                const auto close = pattern.find('}', index + 1);
                if (close == std::string::npos) {
                    diagnostics.error("unterminated definition reference in regex: " + pattern);
                    return result;
                }

                const auto name = pattern.substr(index + 1, close - index - 1);
                if (!is_definition_name(name)) {
                    result.push_back(ch);
                    continue;
                }

                const auto found = definitions.find(name);
                if (found == definitions.end()) {
                    diagnostics.error("unknown regex definition: " + name);
                    return result;
                }
                if (active.count(name) != 0U) {
                    diagnostics.error("cyclic regex definition detected: " + name);
                    return result;
                }

                active.insert(name);
                const auto expanded = normalize_impl(found->second, definitions, active, diagnostics);
                active.erase(name);
                result += "(" + expanded + ")";
                index = close;
                continue;
            }
        }

        result.push_back(ch);
    }

    return result;
}

}  // namespace

std::string RegexNormalizer::normalize(const std::string& pattern,
                                       const std::unordered_map<std::string, std::string>& definitions,
                                       common::DiagnosticEngine& diagnostics) const {
    if (pattern.empty()) {
        diagnostics.warning("attempted to normalize an empty regex");
    }

    std::unordered_set<std::string> active;
    return normalize_impl(pattern, definitions, active, diagnostics);
}

}  // namespace compilerlab::seulex
