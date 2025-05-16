#pragma once
#include <iostream>
#include <print>

#include "AST.hpp"

inline void DeclareDefaultFunctions()
{
    // Functions
    globalScope->Declare("print", std::make_unique<UndefinedExpr>());
    globalScope->Declare("println", std::make_unique<UndefinedExpr>());
    globalScope->Declare("input", std::make_unique<UndefinedExpr>());

    // Structures
    globalScope->Declare("array", std::make_unique<StructDecl>("array"));
}

inline void DefineDefaultFunctions()
{
    globalScope->Get("print") =
        std::make_unique<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            for(const auto& arg : args)
                if(arg)
                    std::visit([](auto&& v)
                    {
                        if constexpr (!std::is_same_v<std::decay_t<decltype(v)>, std::any>)
                            std::print("{}", v);
                    }, *arg);

            return nullptr;
        });

    globalScope->Get("println") =
        std::make_unique<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            for(const auto& arg : args)
                if(arg)
                    std::visit([](auto&& v)
                    {
                        if constexpr (!std::is_same_v<std::decay_t<decltype(v)>, std::any>)
                            std::print("{}", v);
                    }, *arg);

            std::println("");

            return nullptr;
        });

    globalScope->Get("input") =
        std::make_unique<StatementList>([](const auto&, const auto&) -> ValuePtr
        {
            std::string input;
            std::getline(std::cin, input);

            return std::make_shared<Value>(input);
        });

    using ArrayPtr = std::shared_ptr<std::vector<ValuePtr>>;

    auto vec = std::make_shared<std::vector<ValuePtr>>();

    auto array = std::make_unique<StructDecl>("array");
    array->content["data"] = std::make_unique<VariableDecl>("data", std::make_unique<ValueExpr>(std::move(vec)));
    array->content["at"] = std::make_unique<FunctionDecl>("at", std::make_unique<StatementList>(
            [&](const std::vector<ValuePtr>& args, const ScopePtr& scope) -> ValuePtr
            {
                if(args.empty())
                    throw std::runtime_error("Not enough arguments");

                if(args[0])
                {
                    // It's kinda messy
                    const auto evaluated = *scope->Get("this")->Evaluate(scope);
                    const auto self = std::any_cast<StructInstancePtr>(std::get<std::any>(evaluated));
                    const auto data = *self->localScope->Get("data")->Evaluate({});
                    const auto any = std::get<std::any>(*std::any_cast<ValuePtr>(std::get<std::any>(data)));

                    const auto arr = std::any_cast<ArrayPtr>(any);

                    return arr->at(std::get<int>(*args[0]));
                }

                return nullptr;
            }));
    array->content["add"] =
        std::make_unique<FunctionDecl>("add", std::make_unique<StatementList>(
            [&](const std::vector<ValuePtr>& args, const ScopePtr& scope) -> ValuePtr
            {
                if(args.empty())
                    throw std::runtime_error("Not enough arguments");

                const auto evaluated = *scope->Get("this")->Evaluate(scope);
                const auto self = std::any_cast<StructInstancePtr>(std::get<std::any>(evaluated));
                const auto data = *self->localScope->Get("data")->Evaluate({});
                const auto any = std::get<std::any>(*std::any_cast<ValuePtr>(std::get<std::any>(data)));

                const auto arr = std::any_cast<ArrayPtr>(any);

                for(const auto& arg : args)
                    if(arg)
                        arr->push_back(arg);

                return nullptr;
            }));

    globalScope->Get("array") = std::move(array);
}
