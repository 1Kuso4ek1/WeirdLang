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
    ExprPtr ParseUnary();

    ExprPtr ParseReserved();
    ExprPtr ParseIdentifier();
    ExprPtr ParseNumber();
    ExprPtr ParseString();
    ExprPtr ParseStatementList(bool singleExpr = false);
    ExprPtr ParseVarOrFunc(const std::string& token);
    ExprPtr ParseIf();
    ExprPtr ParseWhile();
    ExprPtr ParseFor();
    ExprPtr ParseStruct();

    std::vector<ExprPtr> ParseArguments();

    int GetPrecedence() const;

    void Expect(Lexer::TokenType tokenType, bool skip = true);

private:
    const std::unordered_map<Lexer::TokenType, int> precedence =
    {
        { Lexer::TokenType::Equal, 1 },
        { Lexer::TokenType::AddAssign, 1 },
        { Lexer::TokenType::SubAssign, 1 },
        { Lexer::TokenType::MulAssign, 1 },
        { Lexer::TokenType::DivAssign, 1 },
        { Lexer::TokenType::ModAssign, 1 },
        { Lexer::TokenType::Plus, 2 },
        { Lexer::TokenType::Minus, 2 },
        { Lexer::TokenType::Multiply, 3 },
        { Lexer::TokenType::Divide, 3 },
        { Lexer::TokenType::Modulo, 3 },
        { Lexer::TokenType::And, 1 },
        { Lexer::TokenType::Or, 1 },
        { Lexer::TokenType::BitwiseAnd, 2 },
        { Lexer::TokenType::BitwiseOr, 2 },
        { Lexer::TokenType::BitwiseXor, 2 },
        { Lexer::TokenType::Less, 2 },
        { Lexer::TokenType::Greater, 2 },
        { Lexer::TokenType::LessEqual, 2 },
        { Lexer::TokenType::GreaterEqual, 2 },
        { Lexer::TokenType::Dot, 4 }
    };

private:
    Lexer& lexer;

    Lexer::Token currentToken;
    ExprPtr root{};
};
