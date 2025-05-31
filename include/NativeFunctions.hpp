#pragma once
#include <cstring>
#include <iostream>
#include <print>

#include "AST/AST.hpp"

inline std::any GetFromStruct(const ScopePtr& scope, const std::string& name)
{
    const auto evaluated = *scope->Get("this")->Evaluate(scope);
    const auto self = std::any_cast<StructInstancePtr>(std::get<std::any>(evaluated));
    const auto data = *self->localScope->Get(name)->Evaluate({});

    return std::get<std::any>(*std::any_cast<ValuePtr>(std::get<std::any>(data)));
}

inline void DeclareDefaultFunctions()
{
    // Functions
    globalScope->Declare("print", std::make_shared<UndefinedExpr>());
    globalScope->Declare("println", std::make_shared<UndefinedExpr>());
    globalScope->Declare("input", std::make_shared<UndefinedExpr>());
    globalScope->Declare("strcpy", std::make_shared<UndefinedExpr>());
    globalScope->Declare("strlen", std::make_shared<UndefinedExpr>());
    globalScope->Declare("alloc", std::make_shared<UndefinedExpr>());
    globalScope->Declare("realloc", std::make_shared<UndefinedExpr>());
    globalScope->Declare("free", std::make_shared<UndefinedExpr>());

    // Structures
    globalScope->Declare("array", std::make_shared<StructDecl>("array"));
}

inline void DefineDefaultFunctions()
{
    globalScope->Get("print") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            for(const auto& arg : args)
                std::visit([](auto&& v)
                {
                    if constexpr (std::is_same_v<std::decay_t<decltype(v)>, size_t>)
                        std::print("{}", reinterpret_cast<const char*>(v));
                    else if constexpr (!std::is_same_v<std::decay_t<decltype(v)>, std::any>)
                        std::print("{}", v);
                    else
                        std::print("Non printable");
                }, *arg);

            return nullptr;
        });

    globalScope->Get("println") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            for(const auto& arg : args)
                std::visit([](auto&& v)
                {
                    if constexpr (std::is_same_v<std::decay_t<decltype(v)>, size_t>)
                        std::print("{}", reinterpret_cast<const char*>(v));
                    else if constexpr (!std::is_same_v<std::decay_t<decltype(v)>, std::any>)
                        std::print("{}", v);
                    else
                        std::print("Non printable");
                }, *arg);

            std::println("");

            return nullptr;
        });

    globalScope->Get("input") =
        std::make_shared<StatementList>([](const auto&, const auto&) -> ValuePtr
        {
            std::string input;
            std::getline(std::cin, input);

            return std::make_shared<Value>(input);
        });

    globalScope->Get("strcpy") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            if(args.size() < 2)
                throw std::runtime_error("Not enough arguments");

            const auto dst = reinterpret_cast<char*>(std::get<size_t>(*args[0]));
            const auto src = reinterpret_cast<char*>(std::get<size_t>(*args[1]));

            std::strcpy(dst, src);

            return nullptr;
        });

    globalScope->Get("strlen") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            if(args.empty())
                throw std::runtime_error("Not enough arguments");

            const auto str = reinterpret_cast<char*>(std::get<size_t>(*args[0]));

            return std::make_shared<Value>(static_cast<int>(std::strlen(str)));
        });

    globalScope->Get("alloc") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            if(args.empty())
                throw std::runtime_error("Not enough arguments");

            return std::make_shared<Value>(reinterpret_cast<size_t>(malloc(std::get<int>(*args[0]) * sizeof(Value))));
        });

    globalScope->Get("realloc") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            if(args.size() < 2)
                throw std::runtime_error("Not enough arguments");

            return std::make_shared<Value>(
                reinterpret_cast<size_t>(
                    realloc(
                        reinterpret_cast<void*>(std::get<size_t>(*args[0])),
                        std::get<int>(*args[1]) * sizeof(Value)
                    )
                ));
        });

    globalScope->Get("free") =
        std::make_shared<StatementList>([](const std::vector<ValuePtr>& args, const auto&) -> ValuePtr
        {
            if(args.empty())
                throw std::runtime_error("Not enough arguments");

            free(reinterpret_cast<void*>(std::get<size_t>(*args[0])));

            return nullptr;
        });

    using ArrayPtr = std::shared_ptr<std::vector<ValuePtr>>;

    auto vec = std::make_shared<std::vector<ValuePtr>>();

    auto array = std::make_shared<StructDecl>("array");
    array->content["data"] = std::make_shared<VariableDecl>("data", std::make_shared<ValueExpr>(std::move(vec)));
    array->content["at"] = std::make_shared<FunctionDecl>("at", std::make_shared<StatementList>(
            [&](const std::vector<ValuePtr>& args, const ScopePtr& scope) -> ValuePtr
            {
                if(args.empty())
                    throw std::runtime_error("Not enough arguments");

                const auto arr = std::any_cast<ArrayPtr>(GetFromStruct(scope, "data"));

                return arr->at(std::get<int>(*args[0]));
            }));
    array->content["add"] =
        std::make_shared<FunctionDecl>("add", std::make_shared<StatementList>(
            [&](const std::vector<ValuePtr>& args, const ScopePtr& scope) -> ValuePtr
            {
                if(args.empty())
                    throw std::runtime_error("Not enough arguments");

                const auto arr = std::any_cast<ArrayPtr>(GetFromStruct(scope, "data"));

                for(const auto& arg : args)
                    arr->push_back(std::make_shared<Value>(*arg));

                return nullptr;
            }));
    array->content["size"] =
        std::make_shared<FunctionDecl>("size", std::make_shared<StatementList>(
            [&](const auto&, const ScopePtr& scope) -> ValuePtr
            {
                const auto arr = std::any_cast<ArrayPtr>(GetFromStruct(scope, "data"));

                return std::make_shared<Value>(static_cast<int>(arr->size()));
            }));

    globalScope->Get("array") = std::move(array);
}
