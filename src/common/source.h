#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace compilerlab::common {

struct SourceLocation {
    std::size_t offset {0};
    std::size_t line {1};
    std::size_t column {1};

    bool is_valid() const;
};

struct SourceSpan {
    std::string file_path;
    SourceLocation begin {};
    SourceLocation end {};

    bool is_valid() const;
};

class SourceFile {
public:
    SourceFile() = default;
    SourceFile(std::string path, std::string content);

    const std::string& path() const;
    const std::string& content() const;
    SourceLocation locate(std::size_t offset) const;

private:
    void build_line_index();

    std::string path_;
    std::string content_;
    std::vector<std::size_t> line_offsets_;
};

class SourceManager {
public:
    std::shared_ptr<SourceFile> load_file(const std::string& path);
    std::shared_ptr<SourceFile> add_virtual_file(std::string path, std::string content);
    std::shared_ptr<const SourceFile> get(const std::string& path) const;

private:
    std::unordered_map<std::string, std::shared_ptr<SourceFile>> files_;
};

}  // namespace compilerlab::common
