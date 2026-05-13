#include "common/source.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace compilerlab::common {

bool SourceLocation::is_valid() const {
    return line >= 1 && column >= 1;
}

bool SourceSpan::is_valid() const {
    return begin.is_valid() && end.is_valid();
}

SourceFile::SourceFile(std::string path, std::string content)
    : path_(std::move(path)), content_(std::move(content)) {
    build_line_index();
}

const std::string& SourceFile::path() const {
    return path_;
}

const std::string& SourceFile::content() const {
    return content_;
}

SourceLocation SourceFile::locate(std::size_t offset) const {
    if (line_offsets_.empty()) {
        return {offset, 1, offset + 1};
    }

    const auto clamped = std::min(offset, content_.size());
    const auto upper = std::upper_bound(line_offsets_.begin(), line_offsets_.end(), clamped);
    const auto line_index = upper == line_offsets_.begin()
        ? std::size_t {0}
        : static_cast<std::size_t>(std::distance(line_offsets_.begin(), upper) - 1);
    const auto line_start = line_offsets_[line_index];
    return {clamped, line_index + 1, clamped - line_start + 1};
}

void SourceFile::build_line_index() {
    line_offsets_.clear();
    line_offsets_.push_back(0);
    for (std::size_t index = 0; index < content_.size(); ++index) {
        if (content_[index] == '\n') {
            line_offsets_.push_back(index + 1);
        }
    }
}

std::shared_ptr<SourceFile> SourceManager::load_file(const std::string& path) {
    if (auto existing = files_.find(path); existing != files_.end()) {
        return existing->second;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open source file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    auto file = std::make_shared<SourceFile>(path, buffer.str());
    files_[path] = file;
    return file;
}

std::shared_ptr<SourceFile> SourceManager::add_virtual_file(std::string path, std::string content) {
    auto file = std::make_shared<SourceFile>(std::move(path), std::move(content));
    files_[file->path()] = file;
    return file;
}

std::shared_ptr<const SourceFile> SourceManager::get(const std::string& path) const {
    if (auto it = files_.find(path); it != files_.end()) {
        return it->second;
    }
    return nullptr;
}

}  // namespace compilerlab::common
