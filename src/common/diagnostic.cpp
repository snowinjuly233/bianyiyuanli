#include "common/diagnostic.h"

namespace compilerlab::common {

void DiagnosticEngine::note(std::string message, SourceSpan span) {
    add(DiagnosticSeverity::Note, std::move(message), std::move(span));
}

void DiagnosticEngine::warning(std::string message, SourceSpan span) {
    add(DiagnosticSeverity::Warning, std::move(message), std::move(span));
}

void DiagnosticEngine::error(std::string message, SourceSpan span) {
    add(DiagnosticSeverity::Error, std::move(message), std::move(span));
}

bool DiagnosticEngine::has_error() const {
    for (const auto& item : diagnostics_) {
        if (item.severity == DiagnosticSeverity::Error) {
            return true;
        }
    }
    return false;
}

const std::vector<Diagnostic>& DiagnosticEngine::diagnostics() const {
    return diagnostics_;
}

void DiagnosticEngine::add(DiagnosticSeverity severity, std::string message, SourceSpan span) {
    diagnostics_.push_back({severity, std::move(message), std::move(span)});
}

}  // namespace compilerlab::common
