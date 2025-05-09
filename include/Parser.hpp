#pragma once
#include <unordered_map>

#include "AST.hpp"
#include "Lexer.hpp"

class Parser
{
public:
    explicit Parser(Lexer& lexer);
    ~Parser() = default;

    ExprPtr GetRoot();

private:
    void NextToken();

    ExprPtr Parse();

    ExprPtr ParsePrimary();
    ExprPtr ParseBinaryRight(int leftPrec, ExprPtr left);

    ExprPtr ParseReserved();
    ExprPtr ParseIdentifier();
    ExprPtr ParseNumber();
    ExprPtr ParseString();

    std::vector<ExprPtr> ParseArguments();

    int GetPrecedence() const;

    void Expect(Lexer::TokenType tokenType, bool skip = true);

private:
    const std::unordered_map<Lexer::TokenType, int> precedence =
    {
        { Lexer::TokenType::Equal, 1 },
        { Lexer::TokenType::Plus, 2 },
        { Lexer::TokenType::Minus, 2 },
        { Lexer::TokenType::Multiply, 3 },
        { Lexer::TokenType::Divide, 3 }
    };

private:
    Lexer& lexer;

    Lexer::Token currentToken;
    ExprPtr root{};
};
