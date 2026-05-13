#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/source.h"
#include "semantic/types.h"

namespace compilerlab::semantic {

enum class ScopeKind {
    Global,
    Function,
    Block,
};

struct Symbol {
    std::string name;
    TypePtr type;
    common::SourceSpan span {};
};

class Scope {
public:
    Scope(ScopeKind kind, std::shared_ptr<Scope> parent = nullptr);

    bool insert(Symbol symbol);
    const Symbol* lookup_local(const std::string& name) const;
    const Symbol* lookup(const std::string& name) const;

    ScopeKind kind() const;
    std::shared_ptr<Scope> parent() const;

private:
    ScopeKind kind_;
    std::shared_ptr<Scope> parent_;
    std::unordered_map<std::string, Symbol> symbols_;
};

class SymbolTable {
public:
    SymbolTable();

    void reset();
    void enter_scope(ScopeKind kind);
    void leave_scope();
    bool insert(Symbol symbol);
    const Symbol* lookup(const std::string& name) const;
    const Symbol* lookup_current_scope(const std::string& name) const;

    std::shared_ptr<Scope> current_scope() const;

private:
    std::shared_ptr<Scope> root_;
    std::shared_ptr<Scope> current_;
    std::vector<std::shared_ptr<Scope>> stack_;
};

}  // namespace compilerlab::semantic
