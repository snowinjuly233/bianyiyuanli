#pragma once

#include <string>
#include <vector>

#include "common/diagnostic.h"
#include "runtime/parser_runtime.h"
#include "semantic/ast_factory.h"
#include "semantic/semantic_value.h"

namespace compilerlab::semantic {

class SemanticActionExecutor {
public:
    explicit SemanticActionExecutor(common::DiagnosticEngine& diagnostics);

    SemanticValue execute(const runtime::ProductionInfo& production,
                          const std::vector<SemanticValue>& rhs_values) const;

    common::DiagnosticEngine& diagnostics() const;
    const AstFactory& factory() const;

private:
    common::DiagnosticEngine& diagnostics_;
    AstFactory factory_;
};

}  // namespace compilerlab::semantic
