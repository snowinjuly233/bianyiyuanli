#include "semantic/symbol_table.h"

#include <utility>

namespace compilerlab::semantic {

Scope::Scope(ScopeKind kind, std::shared_ptr<Scope> parent)
    : kind_(kind), parent_(std::move(parent)) {
}

bool Scope::insert(Symbol symbol) {
    const auto [it, inserted] = symbols_.emplace(symbol.name, std::move(symbol));
    return inserted;
}

const Symbol* Scope::lookup_local(const std::string& name) const {
    if (auto it = symbols_.find(name); it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Symbol* Scope::lookup(const std::string& name) const {
    if (const auto* local = lookup_local(name)) {
        return local;
    }
    return parent_ ? parent_->lookup(name) : nullptr;
}

ScopeKind Scope::kind() const {
    return kind_;
}

std::shared_ptr<Scope> Scope::parent() const {
    return parent_;
}

SymbolTable::SymbolTable() {
    reset();
}

void SymbolTable::reset() {
    root_ = std::make_shared<Scope>(ScopeKind::Global);
    current_ = root_;
    stack_.clear();
    stack_.push_back(root_);
}

void SymbolTable::enter_scope(ScopeKind kind) {
    auto next = std::make_shared<Scope>(kind, current_);
    current_ = next;
    stack_.push_back(next);
}

void SymbolTable::leave_scope() {
    if (stack_.size() > 1) {
        stack_.pop_back();
        current_ = stack_.back();
    }
}

bool SymbolTable::insert(Symbol symbol) {
    return current_ ? current_->insert(std::move(symbol)) : false;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    return current_ ? current_->lookup(name) : nullptr;
}

const Symbol* SymbolTable::lookup_current_scope(const std::string& name) const {
    return current_ ? current_->lookup_local(name) : nullptr;
}

std::shared_ptr<Scope> SymbolTable::current_scope() const {
    return current_;
}

}  // namespace compilerlab::semantic
