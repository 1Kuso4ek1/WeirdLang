#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class Lexer
{
public:
    enum class TokenType
    {
        None, Reserved, Identifier, Number, Bool, String,
        Plus, Minus, Multiply, Divide, Modulo, Equal, Semicolon, Comma,
        AddAssign, SubAssign, MulAssign, DivAssign, ModAssign,
        Increment, Decrement,
        And, Or, BitwiseAnd, BitwiseOr, BitwiseXor, Not,
        BitwiseAndAssign, BitwiseOrAssign, BitwiseXorAssign,
        IsEqual, NotEqual, Less, Greater, LessEqual, GreaterEqual, Arrow,
        LeftParen, RightParen, LeftBrace, RightBrace,
        EndOfFile
    };

    using Token = std::pair<TokenType, std::string>;
    using TokenIter = std::vector<Token>::iterator;
    using StringIter = std::string::const_iterator;

    explicit Lexer(std::string&& code);
    ~Lexer() = default;

    Token NextToken();

private:
    void Tokenize();

    Token ProcessIdentifier(StringIter& iter) const;
    static Token ProcessNumber(StringIter& iter);
    static Token ProcessString(StringIter& iter);
    Token ProcessOperator(const StringIter& iter);

private:
    const std::vector<std::string> reservedWords =
    {
        "var", "fun", "if", "else", "while"
    };

private:
    const std::string code;

    bool comment = false;
    std::vector<Token> tokens;
    TokenIter current;
};

const static std::unordered_map<char, Lexer::Token> operatorTokensMap =
{
    { '+', { Lexer::TokenType::Plus, "+" } },
    { '-', { Lexer::TokenType::Minus, "-" } },
    { '*', { Lexer::TokenType::Multiply, "*" } },
    { '/', { Lexer::TokenType::Divide, "/" } },
    { '%', { Lexer::TokenType::Modulo, "%" } },
    { '=', { Lexer::TokenType::Equal, "=" } },
    { '<', { Lexer::TokenType::Less, "<" } },
    { '>', { Lexer::TokenType::Greater, ">" } },
    { '(', { Lexer::TokenType::LeftParen, "(" } },
    { ')', { Lexer::TokenType::RightParen, ")" } },
    { ';', { Lexer::TokenType::Semicolon, ";" } },
    { '{', { Lexer::TokenType::LeftBrace, "{" } },
    { '}', { Lexer::TokenType::RightBrace, "}" } },
    { ',', { Lexer::TokenType::Comma, "," } },
    { '&', { Lexer::TokenType::BitwiseAnd, "&" } },
    { '|', { Lexer::TokenType::BitwiseOr, "|" } },
    { '^', { Lexer::TokenType::BitwiseXor, "^" } },
    { '!', { Lexer::TokenType::Not, "!" } }
};

const static std::map<std::pair<char, char>, Lexer::Token> doubleTokensMap =
{
    { { '+', '=' }, { Lexer::TokenType::AddAssign, "+=" } },
    { { '-', '=' }, { Lexer::TokenType::SubAssign, "-=" } },
    { { '*', '=' }, { Lexer::TokenType::MulAssign, "*=" } },
    { { '/', '=' }, { Lexer::TokenType::DivAssign, "/=" } },
    { { '%', '=' }, { Lexer::TokenType::ModAssign, "%=" } },

    { { '+', '+' }, { Lexer::TokenType::Increment, "++" } },
    { { '-', '-' }, { Lexer::TokenType::Decrement, "--" } },

    { { '&', '&' }, { Lexer::TokenType::And, "&&" } },
    { { '|', '|' }, { Lexer::TokenType::Or, "||" } },
    { { '&', '=' }, { Lexer::TokenType::BitwiseAndAssign, "&=" } },
    { { '|', '=' }, { Lexer::TokenType::BitwiseOrAssign, "|=" } },
    { { '^', '=' }, { Lexer::TokenType::BitwiseXorAssign, "^=" } },

    { { '!', '=' }, { Lexer::TokenType::NotEqual, "!=" } },
    { { '=', '=' }, { Lexer::TokenType::IsEqual, "==" } },
    { { '!', '=' }, { Lexer::TokenType::NotEqual, "!=" } },
    { { '<', '=' }, { Lexer::TokenType::LessEqual, "<=" } },
    { { '>', '=' }, { Lexer::TokenType::GreaterEqual, ">=" } },
    { { '-', '>' }, { Lexer::TokenType::Arrow, "->" } }
};

const static std::unordered_map<Lexer::TokenType, std::string> tokenTypeMap =
{
    { Lexer::TokenType::None, "None" },
    { Lexer::TokenType::Reserved, "Reserved" },
    { Lexer::TokenType::Identifier, "Identifier" },
    { Lexer::TokenType::Number, "Number" },
    { Lexer::TokenType::Bool, "Bool" },
    { Lexer::TokenType::String, "String" },
    { Lexer::TokenType::Plus, "Plus" },
    { Lexer::TokenType::Minus, "Minus" },
    { Lexer::TokenType::Multiply, "Multiply" },
    { Lexer::TokenType::Divide, "Divide" },
    { Lexer::TokenType::Modulo, "Modulo" },
    { Lexer::TokenType::Equal, "Equal" },
    { Lexer::TokenType::Semicolon, "Semicolon" },
    { Lexer::TokenType::Comma, "Comma" },
    { Lexer::TokenType::AddAssign, "AddAssign" },
    { Lexer::TokenType::SubAssign, "SubAssign" },
    { Lexer::TokenType::MulAssign, "MulAssign" },
    { Lexer::TokenType::DivAssign, "DivAssign" },
    { Lexer::TokenType::ModAssign, "ModAssign" },
    { Lexer::TokenType::Increment, "Increment" },
    { Lexer::TokenType::Decrement, "Decrement" },
    { Lexer::TokenType::And, "And" },
    { Lexer::TokenType::Or, "Or" },
    { Lexer::TokenType::Not, "Not" },
    { Lexer::TokenType::BitwiseAnd, "BitwiseAnd" },
    { Lexer::TokenType::BitwiseOr, "BitwiseOr" },
    { Lexer::TokenType::BitwiseXor, "BitwiseXor" },
    { Lexer::TokenType::BitwiseAndAssign, "BitwiseAndAssign" },
    { Lexer::TokenType::BitwiseOrAssign, "BitwiseOrAssign" },
    { Lexer::TokenType::BitwiseXorAssign, "BitwiseXorAssign" },
    { Lexer::TokenType::IsEqual, "IsEqual" },
    { Lexer::TokenType::NotEqual, "NotEqual" },
    { Lexer::TokenType::Less, "Less" },
    { Lexer::TokenType::Greater, "Greater" },
    { Lexer::TokenType::LessEqual, "LessEqual" },
    { Lexer::TokenType::GreaterEqual, "GreaterEqual" },
    { Lexer::TokenType::Arrow, "Arrow" },
    { Lexer::TokenType::LeftParen, "LeftParen" },
    { Lexer::TokenType::RightParen, "RightParen" },
    { Lexer::TokenType::LeftBrace, "LeftBrace" },
    { Lexer::TokenType::RightBrace, "RightBrace" },
    { Lexer::TokenType::EndOfFile, "EndOfFile" }
};

inline std::string TokenTypeToString(const Lexer::TokenType type)
{
    return tokenTypeMap.at(type);
}
