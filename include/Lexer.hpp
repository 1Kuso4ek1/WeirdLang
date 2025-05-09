#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Lexer
{
public:
    enum class TokenType
    {
        None, Reserved, Identifier, Number, Bool, String,
        Plus, Minus, Multiply, Divide, Equal, Semicolon, Comma,
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
    Token ProcessNumber(StringIter& iter) const;
    Token ProcessString(StringIter& iter) const;
    Token ProcessOperator(StringIter& iter) const;

private:
    const std::vector<std::string> reservedWords =
    {
        "var"
    };

private:
    const std::string code;

    bool comment = false;
    std::vector<Token> tokens;
    TokenIter current;
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
    { Lexer::TokenType::Equal, "Equal" },
    { Lexer::TokenType::Semicolon, "Semicolon" },
    { Lexer::TokenType::LeftParen, "LeftParen" },
    { Lexer::TokenType::RightParen, "RightParen" },
    { Lexer::TokenType::LeftBrace, "LeftBrace" },
    { Lexer::TokenType::RightBrace, "RightBrace" },
    { Lexer::TokenType::Comma, "Comma" },
    { Lexer::TokenType::EndOfFile, "EndOfFile" }
};

inline std::string TokenTypeToString(const Lexer::TokenType type)
{
    return tokenTypeMap.at(type);
}
