#include "Parser.hpp"

#include <format>
#include <print>

#include "../include/Lexer.hpp"

Parser::Parser(Lexer& lexer)
    : lexer(lexer)
{
    symbolTable["print"] =
        std::make_unique<StatementList>([](const std::vector<ValuePtr>& args) -> ValuePtr
        {
            for(const auto& arg : args)
                if(arg)
                    std::visit([](auto&& v) { std::print("{}", v); }, *arg);

            return nullptr;
        });

    NextToken();

    std::vector<ExprPtr> statements;

    while(currentToken.first != Lexer::TokenType::EndOfFile)
    {
        if(auto expr = Parse())
            statements.emplace_back(std::move(expr));
    }

    root = std::make_unique<StatementList>(std::move(statements));
}

ExprPtr Parser::GetRoot()
{
    return std::move(root);
}

void Parser::NextToken()
{
    if(currentToken.first != Lexer::TokenType::EndOfFile)
        currentToken = lexer.NextToken();
}

ExprPtr Parser::Parse()
{
    auto left = ParsePrimary();
    auto expr = ParseBinaryRight(0, std::move(left));

    return std::move(expr);
}

ExprPtr Parser::ParsePrimary()
{
    switch(currentToken.first)
    {
    case Lexer::TokenType::Reserved: return ParseReserved();
    case Lexer::TokenType::Identifier: return ParseIdentifier();
    case Lexer::TokenType::Number: return ParseNumber();
    case Lexer::TokenType::String: return ParseString();

    case Lexer::TokenType::Bool:
    {
        auto expr = std::make_unique<ValueExpr>(currentToken.second == "true");

        NextToken();

        return expr;
    }

    case Lexer::TokenType::LeftParen:
    {
        NextToken();
        auto expr = Parse();

        Expect(Lexer::TokenType::RightParen);

        return expr;
    }

    case Lexer::TokenType::Semicolon:
        NextToken();

    default:
        break;
    }

    return nullptr;
}

ExprPtr Parser::ParseBinaryRight(const int leftPrec, ExprPtr left)
{
    while(true)
    {
        const int currentPrec = GetPrecedence();

        if(currentPrec < leftPrec)
            return left;

        auto operation = currentToken;
        NextToken();

        auto right = ParsePrimary();

        if(const int nextPrec = GetPrecedence(); currentPrec < nextPrec)
            right = ParseBinaryRight(currentPrec + 1, std::move(right));

        left = std::make_unique<BinaryExpr>(
            operation, std::move(left), std::move(right)
        );
    }
}

ExprPtr Parser::ParseReserved()
{
    if(currentToken.second == "var")
    {
        NextToken();
        Expect(Lexer::TokenType::Identifier, false);

        const auto name = currentToken.second;
        NextToken();

        if(symbolTable.contains(name))
            throw std::runtime_error("Symbol already exists");

        symbolTable[name] = std::make_unique<ValueExpr>(0);

        return std::make_unique<VariableExpr>(name);
    }

    return nullptr;
}

ExprPtr Parser::ParseIdentifier()
{
    if(symbolTable.contains(currentToken.second))
    {
        auto name = currentToken.second;
        NextToken();
        if(currentToken.first == Lexer::TokenType::LeftParen)
        {
            NextToken();
            auto args = ParseArguments();
            return std::make_unique<FunctionCall>(std::move(name), std::move(args));
        }
        return std::make_unique<VariableExpr>(name);
    }

    throw std::runtime_error("Undefined identifier");
}

ExprPtr Parser::ParseNumber()
{
    const auto valueStr = currentToken.second;
    Value value;

    if(valueStr.back() == 'f')
        value = std::stof(valueStr);
    else if(valueStr.find('.') != std::string::npos)
        value = std::stod(valueStr);
    else
        value = std::stoi(valueStr);

    NextToken();

    return std::make_unique<ValueExpr>(value);
}

ExprPtr Parser::ParseString()
{
    const auto value = currentToken.second;
    NextToken();

    return std::make_unique<ValueExpr>(value);
}

std::vector<ExprPtr> Parser::ParseArguments()
{
    std::vector<ExprPtr> args;
    while(currentToken.first != Lexer::TokenType::RightParen
        && currentToken.first != Lexer::TokenType::EndOfFile)
    {
        args.emplace_back(Parse());
        if(currentToken.first == Lexer::TokenType::Comma)
            NextToken();
    }

    Expect(Lexer::TokenType::RightParen);

    return args;
}

int Parser::GetPrecedence() const
{
    if(precedence.contains(currentToken.first))
        return precedence.at(currentToken.first);

    return -1;
}

void Parser::Expect(Lexer::TokenType tokenType, bool skip)
{
    if(currentToken.first != tokenType)
        throw std::runtime_error(
            std::format(
                "Unexpected token {}. Expected: {}",
                TokenTypeToString(currentToken.first),
                TokenTypeToString(tokenType)
            )
        );

    if(skip)
        NextToken();
}
