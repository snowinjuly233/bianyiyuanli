#include "minic_scanner.h"

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

namespace compilerlab::generated {

std::string to_string(TokenKind kind) {
    switch (kind) {
        case TokenKind::Invalid: return "Invalid";
        case TokenKind::EndOfFile: return "EndOfFile";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::IntegerLiteral: return "IntegerLiteral";
        case TokenKind::FloatLiteral: return "FloatLiteral";
        case TokenKind::KwInt: return "KwInt";
        case TokenKind::KwFloat: return "KwFloat";
        case TokenKind::KwVoid: return "KwVoid";
        case TokenKind::KwIf: return "KwIf";
        case TokenKind::KwElse: return "KwElse";
        case TokenKind::KwWhile: return "KwWhile";
        case TokenKind::KwReturn: return "KwReturn";
        case TokenKind::LParen: return "LParen";
        case TokenKind::RParen: return "RParen";
        case TokenKind::LBrace: return "LBrace";
        case TokenKind::RBrace: return "RBrace";
        case TokenKind::Comma: return "Comma";
        case TokenKind::Semicolon: return "Semicolon";
        case TokenKind::Plus: return "Plus";
        case TokenKind::Minus: return "Minus";
        case TokenKind::Star: return "Star";
        case TokenKind::Slash: return "Slash";
        case TokenKind::Percent: return "Percent";
        case TokenKind::Assign: return "Assign";
        case TokenKind::Equal: return "Equal";
        case TokenKind::NotEqual: return "NotEqual";
        case TokenKind::Less: return "Less";
        case TokenKind::LessEqual: return "LessEqual";
        case TokenKind::Greater: return "Greater";
        case TokenKind::GreaterEqual: return "GreaterEqual";
        case TokenKind::LogicalAnd: return "LogicalAnd";
        case TokenKind::LogicalOr: return "LogicalOr";
        case TokenKind::LogicalNot: return "LogicalNot";
    }
    return "UnknownTokenKind";
}

std::string escape_text(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char ch : text) {
        switch (ch) {
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            case '\\': escaped += "\\\\"; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

SourceLocation MinicScanner::locate(std::string_view input, std::size_t offset) {
    SourceLocation location;
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

std::vector<MinicScanner::ScannerState> MinicScanner::build_states() {
    std::vector<ScannerState> states;
    states.reserve(57);

    states.push_back(ScannerState{0, std::nullopt, {{'\t', 1}, {'\n', 1}, {'\r', 1}, {' ', 1}, {'!', 2}, {'%', 3}, {'&', 4}, {'(', 5}, {')', 6}, {'*', 7}, {'+', 8}, {',', 9}, {'-', 10}, {'/', 11}, {'0', 12}, {'1', 12}, {'2', 12}, {'3', 12}, {'4', 12}, {'5', 12}, {'6', 12}, {'7', 12}, {'8', 12}, {'9', 12}, {';', 13}, {'<', 14}, {'=', 15}, {'>', 16}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 18}, {'f', 19}, {'g', 17}, {'h', 17}, {'i', 20}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 21}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 22}, {'w', 23}, {'x', 17}, {'y', 17}, {'z', 17}, {'{', 24}, {'|', 25}, {'}', 26}}});
    states.push_back(ScannerState{1, ScannerAcceptAction{TokenKind::Invalid, 0, true}, {{'\t', 1}, {'\n', 1}, {'\r', 1}, {' ', 1}}});
    states.push_back(ScannerState{2, ScannerAcceptAction{TokenKind::LogicalNot, 23, false}, {{'=', 27}}});
    states.push_back(ScannerState{3, ScannerAcceptAction{TokenKind::Percent, 28, false}, {}});
    states.push_back(ScannerState{4, std::nullopt, {{'&', 28}}});
    states.push_back(ScannerState{5, ScannerAcceptAction{TokenKind::LParen, 8, false}, {}});
    states.push_back(ScannerState{6, ScannerAcceptAction{TokenKind::RParen, 9, false}, {}});
    states.push_back(ScannerState{7, ScannerAcceptAction{TokenKind::Star, 26, false}, {}});
    states.push_back(ScannerState{8, ScannerAcceptAction{TokenKind::Plus, 24, false}, {}});
    states.push_back(ScannerState{9, ScannerAcceptAction{TokenKind::Comma, 12, false}, {}});
    states.push_back(ScannerState{10, ScannerAcceptAction{TokenKind::Minus, 25, false}, {}});
    states.push_back(ScannerState{11, ScannerAcceptAction{TokenKind::Slash, 27, false}, {}});
    states.push_back(ScannerState{12, ScannerAcceptAction{TokenKind::IntegerLiteral, 30, false}, {{'.', 29}, {'0', 12}, {'1', 12}, {'2', 12}, {'3', 12}, {'4', 12}, {'5', 12}, {'6', 12}, {'7', 12}, {'8', 12}, {'9', 12}}});
    states.push_back(ScannerState{13, ScannerAcceptAction{TokenKind::Semicolon, 13, false}, {}});
    states.push_back(ScannerState{14, ScannerAcceptAction{TokenKind::Less, 21, false}, {{'=', 30}}});
    states.push_back(ScannerState{15, ScannerAcceptAction{TokenKind::Assign, 20, false}, {{'=', 31}}});
    states.push_back(ScannerState{16, ScannerAcceptAction{TokenKind::Greater, 22, false}, {{'=', 32}}});
    states.push_back(ScannerState{17, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{18, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 33}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{19, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 34}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{20, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 35}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 36}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{21, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 37}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{22, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 38}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{23, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 39}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{24, ScannerAcceptAction{TokenKind::LBrace, 10, false}, {}});
    states.push_back(ScannerState{25, std::nullopt, {{'|', 40}}});
    states.push_back(ScannerState{26, ScannerAcceptAction{TokenKind::RBrace, 11, false}, {}});
    states.push_back(ScannerState{27, ScannerAcceptAction{TokenKind::NotEqual, 15, false}, {}});
    states.push_back(ScannerState{28, ScannerAcceptAction{TokenKind::LogicalAnd, 18, false}, {}});
    states.push_back(ScannerState{29, std::nullopt, {{'0', 41}, {'1', 41}, {'2', 41}, {'3', 41}, {'4', 41}, {'5', 41}, {'6', 41}, {'7', 41}, {'8', 41}, {'9', 41}}});
    states.push_back(ScannerState{30, ScannerAcceptAction{TokenKind::LessEqual, 16, false}, {}});
    states.push_back(ScannerState{31, ScannerAcceptAction{TokenKind::Equal, 14, false}, {}});
    states.push_back(ScannerState{32, ScannerAcceptAction{TokenKind::GreaterEqual, 17, false}, {}});
    states.push_back(ScannerState{33, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 42}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{34, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 43}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{35, ScannerAcceptAction{TokenKind::KwIf, 4, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{36, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 44}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{37, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 45}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{38, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 46}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{39, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 47}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{40, ScannerAcceptAction{TokenKind::LogicalOr, 19, false}, {}});
    states.push_back(ScannerState{41, ScannerAcceptAction{TokenKind::FloatLiteral, 29, false}, {{'0', 41}, {'1', 41}, {'2', 41}, {'3', 41}, {'4', 41}, {'5', 41}, {'6', 41}, {'7', 41}, {'8', 41}, {'9', 41}}});
    states.push_back(ScannerState{42, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 48}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{43, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 49}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{44, ScannerAcceptAction{TokenKind::KwInt, 1, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{45, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 50}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{46, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 51}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{47, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 52}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{48, ScannerAcceptAction{TokenKind::KwElse, 5, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{49, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 53}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{50, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 54}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{51, ScannerAcceptAction{TokenKind::KwVoid, 3, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{52, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 55}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{53, ScannerAcceptAction{TokenKind::KwFloat, 2, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{54, ScannerAcceptAction{TokenKind::Identifier, 31, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 56}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{55, ScannerAcceptAction{TokenKind::KwWhile, 6, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});
    states.push_back(ScannerState{56, ScannerAcceptAction{TokenKind::KwReturn, 7, false}, {{'0', 17}, {'1', 17}, {'2', 17}, {'3', 17}, {'4', 17}, {'5', 17}, {'6', 17}, {'7', 17}, {'8', 17}, {'9', 17}, {'A', 17}, {'B', 17}, {'C', 17}, {'D', 17}, {'E', 17}, {'F', 17}, {'G', 17}, {'H', 17}, {'I', 17}, {'J', 17}, {'K', 17}, {'L', 17}, {'M', 17}, {'N', 17}, {'O', 17}, {'P', 17}, {'Q', 17}, {'R', 17}, {'S', 17}, {'T', 17}, {'U', 17}, {'V', 17}, {'W', 17}, {'X', 17}, {'Y', 17}, {'Z', 17}, {'_', 17}, {'a', 17}, {'b', 17}, {'c', 17}, {'d', 17}, {'e', 17}, {'f', 17}, {'g', 17}, {'h', 17}, {'i', 17}, {'j', 17}, {'k', 17}, {'l', 17}, {'m', 17}, {'n', 17}, {'o', 17}, {'p', 17}, {'q', 17}, {'r', 17}, {'s', 17}, {'t', 17}, {'u', 17}, {'v', 17}, {'w', 17}, {'x', 17}, {'y', 17}, {'z', 17}}});

    return states;
}

MinicScanner::MinicScanner(std::string file_path)
    : states_(build_states()), file_path_(std::move(file_path)) {
}

Token MinicScanner::next_token(std::string_view input, std::size_t& offset) const {
    while (true) {
        if (offset == 0 && input.size() >= 3 &&
            static_cast<unsigned char>(input[0]) == 0xEF &&
            static_cast<unsigned char>(input[1]) == 0xBB &&
            static_cast<unsigned char>(input[2]) == 0xBF) {
            offset = 3;
        }

        Token token;
        token.span.file_path = file_path_;
        token.span.begin = locate(input, offset);
        token.span.end = token.span.begin;

        if (offset >= input.size()) {
            token.kind = TokenKind::EndOfFile;
            return token;
        }

        if (states_.empty()) {
            token.kind = TokenKind::Invalid;
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
            const auto& transitions = states_[static_cast<std::size_t>(current_state)].transitions;
            const auto transition = std::find_if(transitions.begin(), transitions.end(),
                [&](const auto& item) { return item.first == input[cursor]; });
            if (transition == transitions.end()) {
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
            token.kind = TokenKind::Invalid;
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

std::vector<Token> MinicScanner::scan_all(std::string_view input) const {
    std::vector<Token> tokens;
    std::size_t offset = 0;
    while (true) {
        auto token = next_token(input, offset);
        tokens.push_back(token);
        if (token.kind == TokenKind::EndOfFile || token.kind == TokenKind::Invalid) {
            break;
        }
    }
    return tokens;
}

}  // namespace compilerlab::generated
