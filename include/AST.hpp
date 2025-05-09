#pragma once
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

struct ExprNode : ASTNode
{
    virtual ValuePtr Evaluate(std::unordered_map<std::string, std::unique_ptr<ExprNode>>& symbolTable) = 0;
};

using ExprPtr = std::unique_ptr<ExprNode>;
using FunctionType = std::function<ValuePtr(const std::vector<ValuePtr>&)>;
using SymbolTable = std::unordered_map<std::string, ExprPtr>;
using FunctionTable = std::unordered_map<std::string, FunctionType>;

inline SymbolTable symbolTable;

inline Value operator+(const Value& lval, const Value& rval)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l + r; }, lval, rval);
}

inline Value operator-(const Value& lval, const Value& rval)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l - r; }, lval, rval);
}

inline Value operator*(const Value& lval, const Value& rval)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l * r; }, lval, rval);
}

inline Value operator/(const Value& lval, const Value& rval)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l / r; }, lval, rval);
}

struct ValueExpr final : ExprNode
{
    explicit ValueExpr(const Value& value)
        : value(std::make_shared<Value>(value))
    {}

    ValuePtr Evaluate(SymbolTable& symbolTable) override
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

    ValuePtr Evaluate(SymbolTable& symbolTable) override
    {
        if(symbolTable.contains(name))
            return symbolTable.at(name)->Evaluate(symbolTable);
        throw std::runtime_error(std::format("Symbol '{}' not found", name));
    }

    std::string name;
};

struct StatementList final : ExprNode
{
    explicit StatementList(std::vector<ExprPtr>&& statements)
        : statements(std::move(statements))
    {}

    explicit StatementList(FunctionType&& nativeFunc)
        : nativeFunc(std::move(nativeFunc))
    {}

    ValuePtr Evaluate(SymbolTable& symbolTable) override
    {
        if(nativeFunc)
        {
            std::vector<ValuePtr> evaluatedArgs;
            evaluatedArgs.reserve(args.size());

            for(const auto& arg : args)
                evaluatedArgs.push_back(arg->Evaluate(symbolTable));

            return nativeFunc(evaluatedArgs);
        }

        // TODO: Make args applicable for custom funcs
        ValuePtr result{};

        for(const auto& i : statements)
            result = i->Evaluate(symbolTable);

        return result;
    }

    FunctionType nativeFunc{};
    std::vector<ExprPtr> args;
    std::vector<ExprPtr> statements;
};

struct FunctionCall final : ExprNode
{
    FunctionCall(std::string&& name, std::vector<ExprPtr>&& args)
        : name(std::move(name)), args(std::move(args))
    {}

    ValuePtr Evaluate(SymbolTable& symbolTable) override
    {
        if(symbolTable.contains(name))
        {
            const auto expr = symbolTable.at(name).get();
            if(const auto cast = dynamic_cast<StatementList*>(expr))
            {
                cast->args = std::move(args);
                return expr->Evaluate(symbolTable);
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

    ValuePtr Evaluate(SymbolTable& symbolTable) override
    {
        auto lval = left->Evaluate(symbolTable);
        auto rval = right->Evaluate(symbolTable);

        switch(token.first)
        {
        case Lexer::TokenType::Equal: *lval = *rval; return lval;
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
