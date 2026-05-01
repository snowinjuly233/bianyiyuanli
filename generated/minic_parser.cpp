#include "minic_parser.h"

#include "common/diagnostic.h"
#include "common/source.h"
#include "common/token.h"
#include "runtime/parser_runtime.h"
#include "semantic/semantic_action_executor.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace compilerlab::generated {

namespace {

using ActionTable = std::vector<std::unordered_map<int, compilerlab::runtime::ParserAction>>;
using GotoTable = std::vector<std::unordered_map<int, int>>;

compilerlab::common::SourceLocation to_common_location(const SourceLocation& location) {
    return compilerlab::common::SourceLocation{location.offset, location.line, location.column};
}

compilerlab::common::SourceSpan to_common_span(const SourceSpan& span) {
    compilerlab::common::SourceSpan converted;
    converted.file_path = span.file_path;
    converted.begin = to_common_location(span.begin);
    converted.end = to_common_location(span.end);
    return converted;
}

SourceLocation to_generated_location(const compilerlab::common::SourceLocation& location) {
    return SourceLocation{location.offset, location.line, location.column};
}

SourceSpan to_generated_span(const compilerlab::common::SourceSpan& span) {
    SourceSpan converted;
    converted.file_path = span.file_path;
    converted.begin = to_generated_location(span.begin);
    converted.end = to_generated_location(span.end);
    return converted;
}

compilerlab::common::TokenKind to_common_token_kind(TokenKind kind) {
    switch (kind) {
        case TokenKind::Identifier: return compilerlab::common::TokenKind::Identifier;
        case TokenKind::IntegerLiteral: return compilerlab::common::TokenKind::IntegerLiteral;
        case TokenKind::FloatLiteral: return compilerlab::common::TokenKind::FloatLiteral;
        case TokenKind::KwInt: return compilerlab::common::TokenKind::KwInt;
        case TokenKind::KwFloat: return compilerlab::common::TokenKind::KwFloat;
        case TokenKind::KwVoid: return compilerlab::common::TokenKind::KwVoid;
        case TokenKind::KwIf: return compilerlab::common::TokenKind::KwIf;
        case TokenKind::KwElse: return compilerlab::common::TokenKind::KwElse;
        case TokenKind::KwWhile: return compilerlab::common::TokenKind::KwWhile;
        case TokenKind::KwReturn: return compilerlab::common::TokenKind::KwReturn;
        case TokenKind::LParen: return compilerlab::common::TokenKind::LParen;
        case TokenKind::RParen: return compilerlab::common::TokenKind::RParen;
        case TokenKind::LBrace: return compilerlab::common::TokenKind::LBrace;
        case TokenKind::RBrace: return compilerlab::common::TokenKind::RBrace;
        case TokenKind::Comma: return compilerlab::common::TokenKind::Comma;
        case TokenKind::Semicolon: return compilerlab::common::TokenKind::Semicolon;
        case TokenKind::Plus: return compilerlab::common::TokenKind::Plus;
        case TokenKind::Minus: return compilerlab::common::TokenKind::Minus;
        case TokenKind::Star: return compilerlab::common::TokenKind::Star;
        case TokenKind::Slash: return compilerlab::common::TokenKind::Slash;
        case TokenKind::Percent: return compilerlab::common::TokenKind::Percent;
        case TokenKind::Assign: return compilerlab::common::TokenKind::Assign;
        case TokenKind::Equal: return compilerlab::common::TokenKind::Equal;
        case TokenKind::NotEqual: return compilerlab::common::TokenKind::NotEqual;
        case TokenKind::Less: return compilerlab::common::TokenKind::Less;
        case TokenKind::LessEqual: return compilerlab::common::TokenKind::LessEqual;
        case TokenKind::Greater: return compilerlab::common::TokenKind::Greater;
        case TokenKind::GreaterEqual: return compilerlab::common::TokenKind::GreaterEqual;
        case TokenKind::LogicalAnd: return compilerlab::common::TokenKind::LogicalAnd;
        case TokenKind::LogicalOr: return compilerlab::common::TokenKind::LogicalOr;
        case TokenKind::LogicalNot: return compilerlab::common::TokenKind::LogicalNot;
        case TokenKind::EndOfFile: return compilerlab::common::TokenKind::EndOfFile;
        default: return compilerlab::common::TokenKind::Invalid;
    }
}

compilerlab::common::Token to_common_token(const Token& token) {
    compilerlab::common::Token converted;
    converted.kind = to_common_token_kind(token.kind);
    converted.lexeme = token.lexeme;
    converted.span = to_common_span(token.span);
    return converted;
}

int token_to_symbol(compilerlab::common::TokenKind kind) {
    switch (kind) {
        case compilerlab::common::TokenKind::Identifier: return 0;
        case compilerlab::common::TokenKind::IntegerLiteral: return 1;
        case compilerlab::common::TokenKind::FloatLiteral: return 2;
        case compilerlab::common::TokenKind::KwInt: return 3;
        case compilerlab::common::TokenKind::KwFloat: return 4;
        case compilerlab::common::TokenKind::KwVoid: return 5;
        case compilerlab::common::TokenKind::KwIf: return 6;
        case compilerlab::common::TokenKind::KwElse: return 7;
        case compilerlab::common::TokenKind::KwWhile: return 8;
        case compilerlab::common::TokenKind::KwReturn: return 9;
        case compilerlab::common::TokenKind::LParen: return 10;
        case compilerlab::common::TokenKind::RParen: return 11;
        case compilerlab::common::TokenKind::LBrace: return 12;
        case compilerlab::common::TokenKind::RBrace: return 13;
        case compilerlab::common::TokenKind::Comma: return 14;
        case compilerlab::common::TokenKind::Semicolon: return 15;
        case compilerlab::common::TokenKind::Plus: return 16;
        case compilerlab::common::TokenKind::Minus: return 17;
        case compilerlab::common::TokenKind::Star: return 18;
        case compilerlab::common::TokenKind::Slash: return 19;
        case compilerlab::common::TokenKind::Percent: return 20;
        case compilerlab::common::TokenKind::Assign: return 21;
        case compilerlab::common::TokenKind::Equal: return 22;
        case compilerlab::common::TokenKind::NotEqual: return 23;
        case compilerlab::common::TokenKind::Less: return 24;
        case compilerlab::common::TokenKind::LessEqual: return 25;
        case compilerlab::common::TokenKind::Greater: return 26;
        case compilerlab::common::TokenKind::GreaterEqual: return 27;
        case compilerlab::common::TokenKind::LogicalAnd: return 28;
        case compilerlab::common::TokenKind::LogicalOr: return 29;
        case compilerlab::common::TokenKind::LogicalNot: return 30;
        case compilerlab::common::TokenKind::EndOfFile: return 32;
        default: return -1;
    }
}

const ActionTable& action_table() {
    static const ActionTable table = [] {
        ActionTable value(116);
        value[0].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[0].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[0].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[1].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 15});
        value[2].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 16});
        value[3].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 17});
        value[4].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 4});
        value[4].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 4});
        value[4].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 4});
        value[4].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 4});
        value[5].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Accept, 0});
        value[6].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[6].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[6].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[6].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 1});
        value[7].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 3});
        value[7].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 3});
        value[7].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 3});
        value[7].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 3});
        value[8].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 5});
        value[8].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 5});
        value[8].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 5});
        value[8].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 5});
        value[9].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 11});
        value[10].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 2});
        value[10].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 2});
        value[10].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 2});
        value[10].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 2});
        value[11].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 12});
        value[11].emplace(21, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 13});
        value[11].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 7});
        value[12].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 10});
        value[12].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[12].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[12].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[13].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[13].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[13].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[13].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[13].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[13].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[14].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 35});
        value[15].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 36});
        value[16].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 37});
        value[16].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 11});
        value[17].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 13});
        value[17].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 13});
        value[18].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 38});
        value[19].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 39});
        value[19].emplace(21, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 40});
        value[19].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[19].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[20].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[20].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 67});
        value[21].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[21].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 68});
        value[22].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[22].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[22].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[22].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[22].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[22].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[23].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[23].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[23].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[23].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[23].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[23].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[24].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[24].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[24].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[24].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[24].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[24].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[25].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 8});
        value[26].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 45});
        value[26].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 46});
        value[26].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 47});
        value[26].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[26].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 54});
        value[27].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 37});
        value[27].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 37});
        value[27].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 37});
        value[28].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 48});
        value[28].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 39});
        value[28].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 39});
        value[28].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 39});
        value[29].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 49});
        value[29].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 41});
        value[29].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 41});
        value[29].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 41});
        value[29].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 41});
        value[30].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 43});
        value[30].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 50});
        value[30].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 43});
        value[30].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 51});
        value[30].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 43});
        value[30].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 43});
        value[30].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 43});
        value[31].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 52});
        value[31].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 53});
        value[31].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 54});
        value[31].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 55});
        value[31].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[31].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 46});
        value[32].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 56});
        value[32].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 57});
        value[32].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[32].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 51});
        value[33].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[33].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 58});
        value[34].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[34].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 61});
        value[35].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 6});
        value[35].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 6});
        value[35].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 6});
        value[35].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 6});
        value[36].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[37].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[37].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[37].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[38].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 14});
        value[38].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 14});
        value[39].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[39].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[39].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[39].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[39].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[39].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[39].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 62});
        value[40].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[40].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[40].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[40].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[40].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[40].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[41].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 65});
        value[42].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 39});
        value[42].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[42].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 66});
        value[43].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[43].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 60});
        value[44].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[44].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 59});
        value[45].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[45].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[45].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[45].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[45].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[45].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[46].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[46].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[46].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[46].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[46].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[46].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[47].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[47].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[47].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[47].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[47].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[47].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[48].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[48].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[48].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[48].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[48].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[48].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[49].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[49].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[49].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[49].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[49].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[49].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[50].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[50].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[50].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[50].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[50].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[50].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[51].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[51].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[51].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[51].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[51].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[51].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[52].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[52].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[52].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[52].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[52].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[52].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[53].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[53].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[53].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[53].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[53].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[53].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[54].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[54].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[54].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[54].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[54].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[54].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[55].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[55].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[55].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[55].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[55].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[55].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[56].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[56].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[56].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[56].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[56].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[56].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[57].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 42});
        value[57].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[57].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[57].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[57].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[57].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[58].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 80});
        value[58].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[58].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[58].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[58].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[58].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[58].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[58].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 79});
        value[58].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 81});
        value[58].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[58].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[58].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 82});
        value[58].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[58].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[58].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 19});
        value[59].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 9});
        value[59].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 9});
        value[59].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 9});
        value[59].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 9});
        value[60].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 12});
        value[60].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 12});
        value[61].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 94});
        value[61].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 63});
        value[62].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 65});
        value[62].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 65});
        value[63].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 95});
        value[64].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 38});
        value[64].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 38});
        value[64].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 38});
        value[65].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[65].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 69});
        value[66].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[66].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 55});
        value[67].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[67].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 56});
        value[68].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[68].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 57});
        value[69].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 49});
        value[69].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 40});
        value[69].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 40});
        value[69].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 40});
        value[69].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 40});
        value[70].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 42});
        value[70].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 50});
        value[70].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 42});
        value[70].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 51});
        value[70].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 42});
        value[70].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 42});
        value[70].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 42});
        value[71].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 52});
        value[71].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 53});
        value[71].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 54});
        value[71].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 55});
        value[71].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[71].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 44});
        value[72].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 52});
        value[72].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 53});
        value[72].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 54});
        value[72].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 55});
        value[72].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[72].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 45});
        value[73].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 56});
        value[73].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 57});
        value[73].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[73].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 47});
        value[74].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 56});
        value[74].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 57});
        value[74].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[74].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 48});
        value[75].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 56});
        value[75].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 57});
        value[75].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[75].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 49});
        value[76].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 56});
        value[76].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 57});
        value[76].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[76].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 50});
        value[77].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 45});
        value[77].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 46});
        value[77].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 47});
        value[77].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[77].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 52});
        value[78].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 45});
        value[78].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 46});
        value[78].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 47});
        value[78].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[78].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 53});
        value[79].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 96});
        value[80].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 97});
        value[81].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[81].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[81].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[81].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[81].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[81].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 98});
        value[81].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[82].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[82].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 31});
        value[83].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 80});
        value[83].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[83].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[83].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[83].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[83].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[83].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[83].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 79});
        value[83].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 81});
        value[83].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[83].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[83].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 82});
        value[83].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[83].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[83].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 20});
        value[84].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 101});
        value[85].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[85].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 26});
        value[86].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[86].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 24});
        value[87].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 102});
        value[88].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[88].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 28});
        value[89].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 103});
        value[90].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[90].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 22});
        value[91].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[91].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 23});
        value[92].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[92].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 25});
        value[93].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[93].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 27});
        value[94].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[94].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[94].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[94].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[94].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[94].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[95].emplace(19, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(16, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(18, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(20, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(22, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(23, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(24, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(25, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(26, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(27, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(28, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[95].emplace(29, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 70});
        value[96].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[96].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[96].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[96].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[96].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[96].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[97].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[97].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[97].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[97].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[97].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[97].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[98].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[98].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 32});
        value[99].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 107});
        value[100].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[100].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 21});
        value[101].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[101].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 30});
        value[102].emplace(21, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 13});
        value[102].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 7});
        value[103].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[103].emplace(32, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 18});
        value[104].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 64});
        value[104].emplace(14, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 64});
        value[105].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 109});
        value[106].emplace(11, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 110});
        value[107].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[107].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 33});
        value[108].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 111});
        value[109].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 80});
        value[109].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[109].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[109].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[109].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[109].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[109].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[109].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 79});
        value[109].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 81});
        value[109].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[109].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[109].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 82});
        value[109].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[109].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[110].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 80});
        value[110].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[110].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[110].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[110].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[110].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[110].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[110].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 79});
        value[110].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 81});
        value[110].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[110].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[110].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 82});
        value[110].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[110].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[111].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[111].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 29});
        value[112].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 114});
        value[112].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[112].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 34});
        value[113].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[113].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 36});
        value[114].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 80});
        value[114].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 19});
        value[114].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 20});
        value[114].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 21});
        value[114].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 1});
        value[114].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 2});
        value[114].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 3});
        value[114].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 79});
        value[114].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 81});
        value[114].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 22});
        value[114].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 58});
        value[114].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 82});
        value[114].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 23});
        value[114].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Shift, 24});
        value[115].emplace(0, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(1, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(2, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(3, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(4, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(5, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(6, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(7, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(8, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(9, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(10, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(12, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(13, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(15, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(17, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        value[115].emplace(30, compilerlab::runtime::ParserAction{compilerlab::runtime::ParserActionKind::Reduce, 35});
        return value;
    }();
    return table;
}

const GotoTable& goto_table() {
    static const GotoTable table = [] {
        GotoTable value(116);
        value[0].emplace(35, 4);
        value[0].emplace(38, 5);
        value[0].emplace(47, 9);
        value[0].emplace(39, 6);
        value[0].emplace(40, 7);
        value[0].emplace(42, 8);
        value[6].emplace(35, 4);
        value[6].emplace(40, 10);
        value[6].emplace(42, 8);
        value[6].emplace(47, 9);
        value[11].emplace(44, 14);
        value[12].emplace(36, 15);
        value[12].emplace(45, 17);
        value[12].emplace(41, 16);
        value[12].emplace(47, 18);
        value[13].emplace(61, 32);
        value[13].emplace(37, 25);
        value[13].emplace(59, 30);
        value[13].emplace(51, 26);
        value[13].emplace(56, 27);
        value[13].emplace(57, 28);
        value[13].emplace(58, 29);
        value[13].emplace(60, 31);
        value[13].emplace(62, 33);
        value[13].emplace(63, 34);
        value[22].emplace(61, 32);
        value[22].emplace(37, 41);
        value[22].emplace(59, 30);
        value[22].emplace(51, 26);
        value[22].emplace(56, 27);
        value[22].emplace(57, 28);
        value[22].emplace(58, 29);
        value[22].emplace(60, 31);
        value[22].emplace(62, 33);
        value[22].emplace(63, 34);
        value[23].emplace(62, 43);
        value[23].emplace(63, 34);
        value[24].emplace(62, 44);
        value[24].emplace(63, 34);
        value[36].emplace(48, 59);
        value[37].emplace(45, 60);
        value[37].emplace(47, 18);
        value[39].emplace(57, 28);
        value[39].emplace(49, 63);
        value[39].emplace(33, 61);
        value[39].emplace(37, 62);
        value[39].emplace(59, 30);
        value[39].emplace(51, 26);
        value[39].emplace(56, 27);
        value[39].emplace(58, 29);
        value[39].emplace(60, 31);
        value[39].emplace(61, 32);
        value[39].emplace(62, 33);
        value[39].emplace(63, 34);
        value[40].emplace(56, 64);
        value[40].emplace(59, 30);
        value[40].emplace(51, 26);
        value[40].emplace(57, 28);
        value[40].emplace(58, 29);
        value[40].emplace(60, 31);
        value[40].emplace(61, 32);
        value[40].emplace(62, 33);
        value[40].emplace(63, 34);
        value[45].emplace(62, 66);
        value[45].emplace(63, 34);
        value[46].emplace(62, 67);
        value[46].emplace(63, 34);
        value[47].emplace(62, 68);
        value[47].emplace(63, 34);
        value[48].emplace(58, 69);
        value[48].emplace(59, 30);
        value[48].emplace(51, 26);
        value[48].emplace(60, 31);
        value[48].emplace(61, 32);
        value[48].emplace(62, 33);
        value[48].emplace(63, 34);
        value[49].emplace(59, 70);
        value[49].emplace(51, 26);
        value[49].emplace(60, 31);
        value[49].emplace(61, 32);
        value[49].emplace(62, 33);
        value[49].emplace(63, 34);
        value[50].emplace(51, 26);
        value[50].emplace(60, 71);
        value[50].emplace(61, 32);
        value[50].emplace(62, 33);
        value[50].emplace(63, 34);
        value[51].emplace(51, 26);
        value[51].emplace(60, 72);
        value[51].emplace(61, 32);
        value[51].emplace(62, 33);
        value[51].emplace(63, 34);
        value[52].emplace(51, 26);
        value[52].emplace(61, 73);
        value[52].emplace(62, 33);
        value[52].emplace(63, 34);
        value[53].emplace(51, 26);
        value[53].emplace(61, 74);
        value[53].emplace(62, 33);
        value[53].emplace(63, 34);
        value[54].emplace(51, 26);
        value[54].emplace(61, 75);
        value[54].emplace(62, 33);
        value[54].emplace(63, 34);
        value[55].emplace(51, 26);
        value[55].emplace(61, 76);
        value[55].emplace(62, 33);
        value[55].emplace(63, 34);
        value[56].emplace(62, 33);
        value[56].emplace(51, 77);
        value[56].emplace(63, 34);
        value[57].emplace(62, 33);
        value[57].emplace(51, 78);
        value[57].emplace(63, 34);
        value[58].emplace(50, 89);
        value[58].emplace(34, 83);
        value[58].emplace(37, 84);
        value[58].emplace(51, 26);
        value[58].emplace(43, 85);
        value[58].emplace(46, 86);
        value[58].emplace(47, 87);
        value[58].emplace(48, 88);
        value[58].emplace(52, 90);
        value[58].emplace(53, 91);
        value[58].emplace(54, 92);
        value[58].emplace(55, 93);
        value[58].emplace(56, 27);
        value[58].emplace(57, 28);
        value[58].emplace(58, 29);
        value[58].emplace(59, 30);
        value[58].emplace(60, 31);
        value[58].emplace(61, 32);
        value[58].emplace(62, 33);
        value[58].emplace(63, 34);
        value[81].emplace(61, 32);
        value[81].emplace(37, 99);
        value[81].emplace(59, 30);
        value[81].emplace(51, 26);
        value[81].emplace(56, 27);
        value[81].emplace(57, 28);
        value[81].emplace(58, 29);
        value[81].emplace(60, 31);
        value[81].emplace(62, 33);
        value[81].emplace(63, 34);
        value[83].emplace(53, 91);
        value[83].emplace(37, 84);
        value[83].emplace(51, 26);
        value[83].emplace(43, 85);
        value[83].emplace(46, 86);
        value[83].emplace(47, 87);
        value[83].emplace(48, 88);
        value[83].emplace(52, 100);
        value[83].emplace(54, 92);
        value[83].emplace(55, 93);
        value[83].emplace(56, 27);
        value[83].emplace(57, 28);
        value[83].emplace(58, 29);
        value[83].emplace(59, 30);
        value[83].emplace(60, 31);
        value[83].emplace(61, 32);
        value[83].emplace(62, 33);
        value[83].emplace(63, 34);
        value[94].emplace(61, 32);
        value[94].emplace(37, 104);
        value[94].emplace(59, 30);
        value[94].emplace(51, 26);
        value[94].emplace(56, 27);
        value[94].emplace(57, 28);
        value[94].emplace(58, 29);
        value[94].emplace(60, 31);
        value[94].emplace(62, 33);
        value[94].emplace(63, 34);
        value[96].emplace(61, 32);
        value[96].emplace(37, 105);
        value[96].emplace(59, 30);
        value[96].emplace(51, 26);
        value[96].emplace(56, 27);
        value[96].emplace(57, 28);
        value[96].emplace(58, 29);
        value[96].emplace(60, 31);
        value[96].emplace(62, 33);
        value[96].emplace(63, 34);
        value[97].emplace(61, 32);
        value[97].emplace(37, 106);
        value[97].emplace(59, 30);
        value[97].emplace(51, 26);
        value[97].emplace(56, 27);
        value[97].emplace(57, 28);
        value[97].emplace(58, 29);
        value[97].emplace(60, 31);
        value[97].emplace(62, 33);
        value[97].emplace(63, 34);
        value[102].emplace(44, 108);
        value[109].emplace(53, 91);
        value[109].emplace(37, 84);
        value[109].emplace(51, 26);
        value[109].emplace(43, 85);
        value[109].emplace(46, 86);
        value[109].emplace(47, 87);
        value[109].emplace(48, 88);
        value[109].emplace(52, 112);
        value[109].emplace(54, 92);
        value[109].emplace(55, 93);
        value[109].emplace(56, 27);
        value[109].emplace(57, 28);
        value[109].emplace(58, 29);
        value[109].emplace(59, 30);
        value[109].emplace(60, 31);
        value[109].emplace(61, 32);
        value[109].emplace(62, 33);
        value[109].emplace(63, 34);
        value[110].emplace(53, 91);
        value[110].emplace(37, 84);
        value[110].emplace(51, 26);
        value[110].emplace(43, 85);
        value[110].emplace(46, 86);
        value[110].emplace(47, 87);
        value[110].emplace(48, 88);
        value[110].emplace(52, 113);
        value[110].emplace(54, 92);
        value[110].emplace(55, 93);
        value[110].emplace(56, 27);
        value[110].emplace(57, 28);
        value[110].emplace(58, 29);
        value[110].emplace(59, 30);
        value[110].emplace(60, 31);
        value[110].emplace(61, 32);
        value[110].emplace(62, 33);
        value[110].emplace(63, 34);
        value[114].emplace(53, 91);
        value[114].emplace(37, 84);
        value[114].emplace(51, 26);
        value[114].emplace(43, 85);
        value[114].emplace(46, 86);
        value[114].emplace(47, 87);
        value[114].emplace(48, 88);
        value[114].emplace(52, 115);
        value[114].emplace(54, 92);
        value[114].emplace(55, 93);
        value[114].emplace(56, 27);
        value[114].emplace(57, 28);
        value[114].emplace(58, 29);
        value[114].emplace(59, 30);
        value[114].emplace(60, 31);
        value[114].emplace(61, 32);
        value[114].emplace(62, 33);
        value[114].emplace(63, 34);
        return value;
    }();
    return table;
}

const std::vector<compilerlab::runtime::ProductionInfo>& productions() {
    static const std::vector<compilerlab::runtime::ProductionInfo> value = {
        compilerlab::runtime::ProductionInfo{64, 1, "__start -> program", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{38, 1, "program -> external_declaration_list", "$$ = make_program($1);"},
        compilerlab::runtime::ProductionInfo{39, 2, "external_declaration_list -> external_declaration_list external_declaration", "$$ = append_decl($1, $2);"},
        compilerlab::runtime::ProductionInfo{39, 1, "external_declaration_list -> external_declaration", "$$ = make_decl_list($1);"},
        compilerlab::runtime::ProductionInfo{40, 1, "external_declaration -> function_definition", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{40, 1, "external_declaration -> global_variable_declaration", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{42, 4, "global_variable_declaration -> type_specifier Identifier initializer_opt Semicolon", "$$ = make_global_var_decl($1, $2, $3);"},
        compilerlab::runtime::ProductionInfo{44, 0, "initializer_opt -> /* empty */", "$$ = make_empty_initializer();"},
        compilerlab::runtime::ProductionInfo{44, 2, "initializer_opt -> Assign expression", "$$ = $2;"},
        compilerlab::runtime::ProductionInfo{35, 6, "function_definition -> type_specifier Identifier LParen parameter_list_opt RParen compound_stmt", "$$ = make_function_decl($1, $2, $4, $6);"},
        compilerlab::runtime::ProductionInfo{36, 0, "parameter_list_opt -> /* empty */", "$$ = make_param_list();"},
        compilerlab::runtime::ProductionInfo{36, 1, "parameter_list_opt -> parameter_list", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{41, 3, "parameter_list -> parameter_list Comma parameter", "$$ = append_param($1, $3);"},
        compilerlab::runtime::ProductionInfo{41, 1, "parameter_list -> parameter", "$$ = make_param_list($1);"},
        compilerlab::runtime::ProductionInfo{45, 2, "parameter -> type_specifier Identifier", "$$ = make_parameter_decl($1, $2);"},
        compilerlab::runtime::ProductionInfo{47, 1, "type_specifier -> KwInt", "$$ = compilerlab::semantic::TypeSpecifier::Int;"},
        compilerlab::runtime::ProductionInfo{47, 1, "type_specifier -> KwFloat", "$$ = compilerlab::semantic::TypeSpecifier::Float;"},
        compilerlab::runtime::ProductionInfo{47, 1, "type_specifier -> KwVoid", "$$ = compilerlab::semantic::TypeSpecifier::Void;"},
        compilerlab::runtime::ProductionInfo{48, 3, "compound_stmt -> LBrace statement_list_opt RBrace", "$$ = make_block_stmt($2);"},
        compilerlab::runtime::ProductionInfo{50, 0, "statement_list_opt -> /* empty */", "$$ = make_stmt_list();"},
        compilerlab::runtime::ProductionInfo{50, 1, "statement_list_opt -> statement_list", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{34, 2, "statement_list -> statement_list statement", "$$ = append_stmt($1, $2);"},
        compilerlab::runtime::ProductionInfo{34, 1, "statement_list -> statement", "$$ = make_stmt_list($1);"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> declaration_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> expression_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> return_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> if_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> while_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{52, 1, "statement -> compound_stmt", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{53, 4, "declaration_stmt -> type_specifier Identifier initializer_opt Semicolon", "$$ = make_local_decl_stmt($1, $2, $3);"},
        compilerlab::runtime::ProductionInfo{46, 2, "expression_stmt -> expression Semicolon", "$$ = make_expr_stmt($1);"},
        compilerlab::runtime::ProductionInfo{46, 1, "expression_stmt -> Semicolon", "$$ = make_empty_expr_stmt();"},
        compilerlab::runtime::ProductionInfo{54, 2, "return_stmt -> KwReturn Semicolon", "$$ = make_return_stmt(nullptr);"},
        compilerlab::runtime::ProductionInfo{54, 3, "return_stmt -> KwReturn expression Semicolon", "$$ = make_return_stmt($2);"},
        compilerlab::runtime::ProductionInfo{43, 5, "if_stmt -> KwIf LParen expression RParen statement", "$$ = make_if_stmt($3, $5, nullptr);"},
        compilerlab::runtime::ProductionInfo{43, 7, "if_stmt -> KwIf LParen expression RParen statement KwElse statement", "$$ = make_if_stmt($3, $5, $7);"},
        compilerlab::runtime::ProductionInfo{55, 5, "while_stmt -> KwWhile LParen expression RParen statement", "$$ = make_while_stmt($3, $5);"},
        compilerlab::runtime::ProductionInfo{37, 1, "expression -> assignment_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{56, 3, "assignment_expression -> Identifier Assign assignment_expression", "$$ = make_assign_expr($1, $3);"},
        compilerlab::runtime::ProductionInfo{56, 1, "assignment_expression -> logical_or_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{57, 3, "logical_or_expression -> logical_or_expression LogicalOr logical_and_expression", "$$ = make_binary_expr(LogicalOr, $1, $3);"},
        compilerlab::runtime::ProductionInfo{57, 1, "logical_or_expression -> logical_and_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{58, 3, "logical_and_expression -> logical_and_expression LogicalAnd equality_expression", "$$ = make_binary_expr(LogicalAnd, $1, $3);"},
        compilerlab::runtime::ProductionInfo{58, 1, "logical_and_expression -> equality_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{59, 3, "equality_expression -> equality_expression Equal relational_expression", "$$ = make_binary_expr(Equal, $1, $3);"},
        compilerlab::runtime::ProductionInfo{59, 3, "equality_expression -> equality_expression NotEqual relational_expression", "$$ = make_binary_expr(NotEqual, $1, $3);"},
        compilerlab::runtime::ProductionInfo{59, 1, "equality_expression -> relational_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{60, 3, "relational_expression -> relational_expression Less additive_expression", "$$ = make_binary_expr(Less, $1, $3);"},
        compilerlab::runtime::ProductionInfo{60, 3, "relational_expression -> relational_expression LessEqual additive_expression", "$$ = make_binary_expr(LessEqual, $1, $3);"},
        compilerlab::runtime::ProductionInfo{60, 3, "relational_expression -> relational_expression Greater additive_expression", "$$ = make_binary_expr(Greater, $1, $3);"},
        compilerlab::runtime::ProductionInfo{60, 3, "relational_expression -> relational_expression GreaterEqual additive_expression", "$$ = make_binary_expr(GreaterEqual, $1, $3);"},
        compilerlab::runtime::ProductionInfo{60, 1, "relational_expression -> additive_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{61, 3, "additive_expression -> additive_expression Plus multiplicative_expression", "$$ = make_binary_expr(Plus, $1, $3);"},
        compilerlab::runtime::ProductionInfo{61, 3, "additive_expression -> additive_expression Minus multiplicative_expression", "$$ = make_binary_expr(Minus, $1, $3);"},
        compilerlab::runtime::ProductionInfo{61, 1, "additive_expression -> multiplicative_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{51, 3, "multiplicative_expression -> multiplicative_expression Star unary_expression", "$$ = make_binary_expr(Star, $1, $3);"},
        compilerlab::runtime::ProductionInfo{51, 3, "multiplicative_expression -> multiplicative_expression Slash unary_expression", "$$ = make_binary_expr(Slash, $1, $3);"},
        compilerlab::runtime::ProductionInfo{51, 3, "multiplicative_expression -> multiplicative_expression Percent unary_expression", "$$ = make_binary_expr(Percent, $1, $3);"},
        compilerlab::runtime::ProductionInfo{51, 1, "multiplicative_expression -> unary_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{62, 2, "unary_expression -> LogicalNot unary_expression", "$$ = make_unary_expr(LogicalNot, $2);"},
        compilerlab::runtime::ProductionInfo{62, 2, "unary_expression -> Minus unary_expression", "$$ = make_unary_expr(Minus, $2);"},
        compilerlab::runtime::ProductionInfo{62, 1, "unary_expression -> primary_expression", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{49, 0, "argument_list_opt -> /* empty */", "$$ = make_expr_list();"},
        compilerlab::runtime::ProductionInfo{49, 1, "argument_list_opt -> argument_list", "$$ = $1;"},
        compilerlab::runtime::ProductionInfo{33, 3, "argument_list -> argument_list Comma expression", "$$ = append_expr($1, $3);"},
        compilerlab::runtime::ProductionInfo{33, 1, "argument_list -> expression", "$$ = make_expr_list($1);"},
        compilerlab::runtime::ProductionInfo{63, 1, "primary_expression -> Identifier", "$$ = make_identifier_expr($1);"},
        compilerlab::runtime::ProductionInfo{63, 1, "primary_expression -> IntegerLiteral", "$$ = make_integer_literal_expr($1);"},
        compilerlab::runtime::ProductionInfo{63, 1, "primary_expression -> FloatLiteral", "$$ = make_float_literal_expr($1);"},
        compilerlab::runtime::ProductionInfo{63, 3, "primary_expression -> LParen expression RParen", "$$ = $2;"},
        compilerlab::runtime::ProductionInfo{63, 4, "primary_expression -> Identifier LParen argument_list_opt RParen", "$$ = make_call_expr($1, $3);"},
    };
    return value;
}

void copy_first_error(const compilerlab::common::DiagnosticEngine& diagnostics, ParseResult& result) {
    for (const auto& item : diagnostics.diagnostics()) {
        if (item.severity == compilerlab::common::DiagnosticSeverity::Error) {
            result.error_message = item.message;
            result.error_span = to_generated_span(item.span);
            return;
        }
    }
}

}  // namespace

std::size_t MinicParser::lr1_state_count() {
    return 253;
}

std::size_t MinicParser::lalr_state_count() {
    return 116;
}

ParseResult MinicParser::parse_source(std::string_view input, const MinicScanner& scanner) const {
    return parse(scanner.scan_all(input));
}

ParseResult MinicParser::parse(const std::vector<Token>& tokens) const {
    ParseResult result;
    if (tokens.empty()) {
        result.error_message = "parser received an empty token stream";
        return result;
    }

    std::vector<compilerlab::common::Token> common_tokens;
    common_tokens.reserve(tokens.size());
    for (const auto& token : tokens) {
        common_tokens.push_back(to_common_token(token));
    }

    compilerlab::common::DiagnosticEngine diagnostics;
    compilerlab::runtime::ParserRuntime parser_runtime;
    compilerlab::semantic::SemanticActionExecutor action_executor(diagnostics);
    const auto& production_list = productions();
    const auto semantic_result = parser_runtime.parse(
        common_tokens,
        action_table(),
        goto_table(),
        production_list,
        [](compilerlab::common::TokenKind kind) {
            return token_to_symbol(kind);
        },
        [&](int production_index, const std::vector<compilerlab::semantic::SemanticValue>& rhs_values) {
            if (production_index > 0 &&
                static_cast<std::size_t>(production_index) < production_list.size()) {
                result.reductions.push_back(
                    "reduce " + std::to_string(result.reductions.size() + 1) + ": " +
                    production_list[static_cast<std::size_t>(production_index)].debug_name);
            }
            return action_executor.execute(
                production_list[static_cast<std::size_t>(production_index)], rhs_values);
        },
        diagnostics);

    if (diagnostics.has_error()) {
        copy_first_error(diagnostics, result);
        return result;
    }

    if (const auto* program = semantic_result.get_if<compilerlab::semantic::ProgramPtr>();
        program != nullptr) {
        result.program = *program;
    }
    result.accepted = true;
    return result;
}

}  // namespace compilerlab::generated
