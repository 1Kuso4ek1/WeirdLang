#pragma once
#include <print>

#include "AST.hpp"

inline void DeclareDefaultFunctions()
{
    globalScope->Declare("print", std::make_unique<UndefinedExpr>());
    globalScope->Declare("println", std::make_unique<UndefinedExpr>());
}

inline void DefineDefaultFunctions()
{
    globalScope->Get("print") =
        std::make_unique<StatementList>([](const std::vector<ValuePtr>& args) -> ValuePtr
        {
            for(const auto& arg : args)
                if(arg)
                    std::visit([](auto&& v) { std::print("{}", v); }, *arg);

            return nullptr;
        });

    globalScope->Get("println") =
        std::make_unique<StatementList>([](const std::vector<ValuePtr>& args) -> ValuePtr
        {
            for(const auto& arg : args)
                if(arg)
                    std::visit([](auto&& v) { std::print("{}", v); }, *arg);

            std::println("");

            return nullptr;
        });
}
