#pragma once

#include <string>
#include <vector>

#include "common/source.h"

namespace compilerlab::common {

enum class DiagnosticSeverity {
    Note,
    Warning,
    Error,
};

struct Diagnostic {
    DiagnosticSeverity severity {DiagnosticSeverity::Note};
    std::string message;
    SourceSpan span {};
};

class DiagnosticEngine {
public:
    void note(std::string message, SourceSpan span = {});
    void warning(std::string message, SourceSpan span = {});
    void error(std::string message, SourceSpan span = {});

    bool has_error() const;
    const std::vector<Diagnostic>& diagnostics() const;

private:
    void add(DiagnosticSeverity severity, std::string message, SourceSpan span);

    std::vector<Diagnostic> diagnostics_;
};

}  // namespace compilerlab::common
