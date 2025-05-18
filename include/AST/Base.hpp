#pragma once
#include <format>
#include <functional>
#include <memory>

#include "Value.hpp"

struct ASTNode
{
    ASTNode() = default;
    virtual ~ASTNode() = default;
};

struct Scope;

struct ExprNode : ASTNode
{
    virtual ValuePtr Evaluate(std::shared_ptr<Scope>) = 0;
    virtual std::shared_ptr<ExprNode> Clone(const std::shared_ptr<Scope> scope) const
    {
        throw std::runtime_error("Expression is not cloneable");
    };
};

struct UndefinedExpr final : ExprNode
{
    ValuePtr Evaluate(const std::shared_ptr<Scope> scope) override
    {
        throw std::runtime_error("Evaluation of an undefined expression");
    }
};

using ExprPtr = std::shared_ptr<ExprNode>;
using SymbolTable = std::unordered_map<std::string, ExprPtr>;
