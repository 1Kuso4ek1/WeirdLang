#pragma once
#include "Base.hpp"

struct Scope
{
    explicit Scope(const std::shared_ptr<Scope>& parent = {})
        : parent(parent)
    {}

    void Declare(const std::string& name, ExprPtr value)
    {
        symbols[name] = std::move(value);
    }

    void Reset()
    {
        symbols.clear();
    }

    ExprPtr& Get(const std::string& name)
    {
        if(const auto it = symbols.find(name); it != symbols.end())
            return it->second;

        if(const auto ptr = parent.lock())
        {
            // Some kind of caching
            const auto& result = ptr->Get(name);
            return symbols[name] = result;
        }

        throw std::runtime_error(std::format("Symbol '{}' not found", name));
    }

    bool Contains(const std::string& name) const
    {
        const auto ptr = parent.lock();
        return symbols.contains(name) || (ptr && ptr->Contains(name));
    }

    std::weak_ptr<Scope> parent;
    SymbolTable symbols;
};

using ScopePtr = std::shared_ptr<Scope>;

inline auto globalScope = std::make_shared<Scope>();
