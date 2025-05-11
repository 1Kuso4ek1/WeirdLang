#pragma once
#include <iostream>
#include <print>

#include "AST.hpp"

inline void DeclareDefaultFunctions()
{
    globalScope->Declare("print", std::make_unique<UndefinedExpr>());
    globalScope->Declare("println", std::make_unique<UndefinedExpr>());
    globalScope->Declare("input", std::make_unique<UndefinedExpr>());
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

    globalScope->Get("input") =
        std::make_unique<StatementList>([](const auto&) -> ValuePtr
        {
            std::string input;
            std::getline(std::cin, input);

            return std::make_shared<Value>(input);
        });
}
