#pragma once
#include <complex>
#include <format>
#include <functional>
#include <memory>
#include <utility>
#include <variant>

#include "Lexer.hpp"

using Value = std::variant<int, float, double, bool, std::string>;
using ValuePtr = std::shared_ptr<Value>;

struct ASTNode
{
    ASTNode() = default;
    virtual ~ASTNode() = default;
};

struct Scope;

struct ExprNode : ASTNode
{
    virtual ValuePtr Evaluate(std::shared_ptr<Scope>) = 0;
};

using ExprPtr = std::unique_ptr<ExprNode>;
using FunctionType = std::function<ValuePtr(const std::vector<ValuePtr>&)>;
using SymbolTable = std::unordered_map<std::string, ExprPtr>;
using FunctionTable = std::unordered_map<std::string, FunctionType>;

struct UndefinedExpr final : ExprNode
{
    ValuePtr Evaluate(std::shared_ptr<Scope> scope) override
    {
        throw std::runtime_error("Evaluation of an undefined expression");
    }
};

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
        if(symbols.contains(name))
            return symbols.at(name);

        if(parent)
            return parent->Get(name);

        throw std::runtime_error("Symbol '" + name + "' not found");
    }

    bool Contains(const std::string& name) const
    {
        return symbols.contains(name) || (parent && parent->Contains(name));
    }

    std::shared_ptr<Scope> parent;
    SymbolTable symbols;
};

inline auto globalScope = std::make_shared<Scope>();

inline Value operator+(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l + r; }, left, right);
}

inline Value operator-(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l - r; }, left, right);
}

inline Value operator*(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l * r; }, left, right);
}

inline Value operator/(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l / r; }, left, right);
}

struct ValueExpr final : ExprNode
{
    explicit ValueExpr(const Value& value)
        : value(std::make_shared<Value>(value))
    {}

    ValuePtr Evaluate(std::shared_ptr<Scope> scope) override
    {
        return value;
    }

    ValuePtr value;
};

struct VariableExpr final : ExprNode
{
    explicit VariableExpr(std::string name)
        : name(std::move(name))
    {}

    ValuePtr Evaluate(const std::shared_ptr<Scope> scope) override
    {
        if(scope->Contains(name))
            return scope->Get(name)->Evaluate(scope);
        throw std::runtime_error(std::format("Symbol '{}' not found", name));
    }

    std::string name;
};

struct VariableDecl final : ExprNode
{
    VariableDecl(const std::string& name, ExprPtr value)
        : name(name), value(std::move(value))
    {}

    ValuePtr Evaluate(const std::shared_ptr<Scope> scope) override
    {
        const auto evaluated = value->Evaluate(scope);
        scope->Declare(name, std::move(value));

        return evaluated;
    }

    std::string name;
    ExprPtr value;
};

struct StatementList final : ExprNode
{
    explicit StatementList(std::vector<ExprPtr>&& statements, std::vector<ExprPtr>&& args = {})
        : statements(std::move(statements)), args(std::move(args))
    {}

    explicit StatementList(FunctionType&& nativeFunc)
        : nativeFunc(std::move(nativeFunc))
    {}

    ValuePtr Evaluate(std::shared_ptr<Scope> scope) override
    {
        if(nativeFunc)
        {
            std::vector<ValuePtr> evaluatedArgs;
            evaluatedArgs.reserve(passedArgs.size());

            for(const auto& arg : passedArgs)
                evaluatedArgs.push_back(arg->Evaluate(scope));

            return nativeFunc(evaluatedArgs);
        }

        const auto localScope = std::make_shared<Scope>(scope);
        for(int i = 0; i < args.size(); i++)
        {
            if(passedArgs.size() < i + 1)
                throw std::runtime_error("Not enough arguments");

            const auto& argName = dynamic_cast<VariableDecl*>(args[i].get())->name;
            localScope->Declare(argName, std::move(passedArgs[i]));
        }

        ValuePtr result{};

        for(const auto& i : statements)
            result = i->Evaluate(localScope);

        return result;
    }

    FunctionType nativeFunc{};
    std::vector<ExprPtr> statements, args, passedArgs;
};

struct FunctionCall final : ExprNode
{
    FunctionCall(std::string&& name, std::vector<ExprPtr>&& args)
        : name(std::move(name)), args(std::move(args))
    {}

    ValuePtr Evaluate(std::shared_ptr<Scope> scope) override
    {
        if(scope->Contains(name))
        {
            const auto expr = scope->Get(name).get();
            if(const auto cast = dynamic_cast<StatementList*>(expr))
            {
                cast->passedArgs = std::move(args);
                return expr->Evaluate(scope);
            }

            throw std::runtime_error(std::format("'{}' is not a function", name));
        }

        throw std::runtime_error(std::format("Function '{}' not found", name));
    }

    std::string name;
    std::vector<ExprPtr> args;
};

struct BinaryExpr final : ExprNode
{
    BinaryExpr(Lexer::Token token, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right)
        : token(std::move(token)), left(std::move(left)), right(std::move(right))
    {}

    ValuePtr Evaluate(const std::shared_ptr<Scope> scope) override
    {
        auto lval = left->Evaluate(scope);
        const auto rval = right->Evaluate(scope);

        switch(token.first)
        {
        case Lexer::TokenType::Equal: *lval = *rval; return lval;
        case Lexer::TokenType::AddAssign: *lval = *lval + *rval; return lval;
        case Lexer::TokenType::SubAssign: *lval = *lval - *rval; return lval;
        case Lexer::TokenType::MulAssign: *lval = *lval * *rval; return lval;
        case Lexer::TokenType::DivAssign: *lval = *lval / *rval; return lval;
        case Lexer::TokenType::Plus: return std::make_shared<Value>(*lval + *rval);
        case Lexer::TokenType::Minus: return std::make_shared<Value>(*lval - *rval);
        case Lexer::TokenType::Multiply: return std::make_shared<Value>(*lval * *rval);
        case Lexer::TokenType::Divide: return std::make_shared<Value>(*lval / *rval);
        default: break;
        }

        return std::make_shared<Value>(0);
    }

    Lexer::Token token;

    ExprPtr left, right;
};
