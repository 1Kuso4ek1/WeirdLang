#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::operator ""s;
using std::operator ""sv;

// Not really the single responsibility class
// It loads and preprocesses the code (finds, loads and inserts imported files)
class Lexer
{
public:
    enum class TokenType
    {
        None, Reserved, Identifier, Number, Bool, Char, String,
        Plus, Minus, Multiply, Divide, Modulo, Equal,
        Semicolon, Comma, Dot,
        AddAssign, SubAssign, MulAssign, DivAssign, ModAssign,
        Increment, Decrement,
        And, Or, BitwiseAnd, BitwiseOr, BitwiseXor, Not, Pointer,
        BitwiseAndAssign, BitwiseOrAssign, BitwiseXorAssign,
        IsEqual, NotEqual, Less, Greater, LessEqual, GreaterEqual, Arrow,
        LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket,
        EndOfFile
    };

    using Token = std::pair<TokenType, std::string>;
    using TokenIter = std::vector<Token>::iterator;
    using StringIter = std::string::const_iterator;

    explicit Lexer(const std::filesystem::path& path);
    ~Lexer() = default;

    Token NextToken();

private:
    void Tokenize();

    Token ProcessIdentifier(StringIter& iter) const;
    static Token ProcessNumber(StringIter& iter);
    static Token ProcessString(StringIter& iter);
    Token ProcessOperator(const StringIter& iter);

private:
    static std::string LoadCode(const std::filesystem::path& path);

    static char ProcessChar(StringIter& iter);

private:
    const std::vector<std::string_view> reservedWords =
    {
        "var"sv, "fun"sv, "if"sv, "else"sv, "while"sv, "for"sv,
        "return"sv, "break"sv, "continue"sv, "struct"sv, "import"sv
    };

private:
    const std::string code;

    bool comment{}, importFilename{};
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
    { ',', { Lexer::TokenType::Comma, "," } },
    { '.', { Lexer::TokenType::Dot, "." } },
    { '{', { Lexer::TokenType::LeftBrace, "{" } },
    { '}', { Lexer::TokenType::RightBrace, "}" } },
    { '[', { Lexer::TokenType::LeftBracket, "[" } },
    { ']', { Lexer::TokenType::RightBracket, "]" } },
    { '&', { Lexer::TokenType::BitwiseAnd, "&" } },
    { '|', { Lexer::TokenType::BitwiseOr, "|" } },
    { '^', { Lexer::TokenType::BitwiseXor, "^" } },
    { '!', { Lexer::TokenType::Not, "!" } },
    { '$', { Lexer::TokenType::Pointer, "$" } }
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

const static std::unordered_map<Lexer::TokenType, std::string_view> tokenTypeMap =
{
    { Lexer::TokenType::None, "None"sv },
    { Lexer::TokenType::Reserved, "Reserved"sv },
    { Lexer::TokenType::Identifier, "Identifier"sv },
    { Lexer::TokenType::Number, "Number"sv },
    { Lexer::TokenType::Bool, "Bool"sv },
    { Lexer::TokenType::Char, "Char"sv },
    { Lexer::TokenType::String, "String"sv },
    { Lexer::TokenType::Plus, "Plus"sv },
    { Lexer::TokenType::Minus, "Minus"sv },
    { Lexer::TokenType::Multiply, "Multiply"sv },
    { Lexer::TokenType::Divide, "Divide"sv },
    { Lexer::TokenType::Modulo, "Modulo"sv },
    { Lexer::TokenType::Equal, "Equal"sv },
    { Lexer::TokenType::Semicolon, "Semicolon"sv },
    { Lexer::TokenType::Comma, "Comma"sv },
    { Lexer::TokenType::Dot, "Dot"sv },
    { Lexer::TokenType::AddAssign, "AddAssign"sv },
    { Lexer::TokenType::SubAssign, "SubAssign"sv },
    { Lexer::TokenType::MulAssign, "MulAssign"sv },
    { Lexer::TokenType::DivAssign, "DivAssign"sv },
    { Lexer::TokenType::ModAssign, "ModAssign"sv },
    { Lexer::TokenType::Increment, "Increment"sv },
    { Lexer::TokenType::Decrement, "Decrement"sv },
    { Lexer::TokenType::And, "And"sv },
    { Lexer::TokenType::Or, "Or"sv },
    { Lexer::TokenType::Not, "Not"sv },
    { Lexer::TokenType::BitwiseAnd, "BitwiseAnd"sv },
    { Lexer::TokenType::BitwiseOr, "BitwiseOr"sv },
    { Lexer::TokenType::BitwiseXor, "BitwiseXor"sv },
    { Lexer::TokenType::BitwiseAndAssign, "BitwiseAndAssign"sv },
    { Lexer::TokenType::BitwiseOrAssign, "BitwiseOrAssign"sv },
    { Lexer::TokenType::BitwiseXorAssign, "BitwiseXorAssign"sv },
    { Lexer::TokenType::IsEqual, "IsEqual"sv },
    { Lexer::TokenType::NotEqual, "NotEqual"sv },
    { Lexer::TokenType::Less, "Less"sv },
    { Lexer::TokenType::Greater, "Greater"sv },
    { Lexer::TokenType::LessEqual, "LessEqual"sv },
    { Lexer::TokenType::GreaterEqual, "GreaterEqual"sv },
    { Lexer::TokenType::Arrow, "Arrow"sv },
    { Lexer::TokenType::LeftParen, "LeftParen"sv },
    { Lexer::TokenType::RightParen, "RightParen"sv },
    { Lexer::TokenType::LeftBrace, "LeftBrace"sv },
    { Lexer::TokenType::RightBrace, "RightBrace"sv },
    { Lexer::TokenType::EndOfFile, "EndOfFile"sv }
};

inline std::string_view TokenTypeToString(const Lexer::TokenType type)
{
    return tokenTypeMap.at(type);
}
