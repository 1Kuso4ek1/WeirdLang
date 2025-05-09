#include "Lexer.hpp"

#include <ranges>

using std::operator ""s;

Lexer::Lexer(std::string&& code)
    : code(std::move(code))
{
    Tokenize();

    current = tokens.begin();
}

Lexer::Token Lexer::NextToken()
{
    return *current++;
}

void Lexer::Tokenize()
{
    for(auto i = code.begin(); i < code.end(); ++i)
    {
        if(std::isspace(*i) || (comment && *i != '#'))
            continue;

        if(*i == '#')
            comment = !comment;
        else if(std::isalpha(*i) || *i == '_')
            tokens.emplace_back(ProcessIdentifier(i));
        else if(std::isdigit(*i))
            tokens.emplace_back(ProcessNumber(i));
        else if(*i == '"')
            tokens.emplace_back(ProcessString(++i));
        else
            tokens.emplace_back(ProcessOperator(i));
    }

    tokens.emplace_back(TokenType::EndOfFile, "");
}

Lexer::Token Lexer::ProcessIdentifier(StringIter& iter) const
{
    auto value = ""s;

    while(std::isalnum(*iter) || *iter == '_')
        value += *iter++;

    --iter;

    if(value == "true" || value == "false")
        return { TokenType::Bool, value };

    if(std::ranges::find(reservedWords, value) != reservedWords.end())
        return { TokenType::Reserved, value };

    return { TokenType::Identifier, value };
}

Lexer::Token Lexer::ProcessNumber(StringIter& iter) const
{
    auto value = ""s;

    bool floatingPoint = false;

    while(std::isdigit(*iter)
        || (*iter == '.' && std::isdigit(*(iter + 1)))
        || (*iter == 'f' && !floatingPoint))
        value += *iter++;

    floatingPoint = (value.back() == 'f') && !floatingPoint;

    --iter;

    return { TokenType::Number, value };
}

Lexer::Token Lexer::ProcessString(StringIter& iter) const
{
    auto value = ""s;

    while(*iter != '"')
        value += *iter++;

    return { TokenType::String, value };
}

Lexer::Token Lexer::ProcessOperator(StringIter& iter) const
{
    switch(*iter)
    {
    case '+': return { TokenType::Plus, "+" };
    case '-': return { TokenType::Minus, "-" };
    case '*': return { TokenType::Multiply, "*" };
    case '/': return { TokenType::Divide, "/" };
    case '=': return { TokenType::Equal, "=" };
    case '(': return { TokenType::LeftParen, "(" };
    case ')': return { TokenType::RightParen, ")" };
    case ';': return { TokenType::Semicolon, ";" };
    case '{': return { TokenType::LeftBrace, "{" };
    case '}': return { TokenType::RightBrace, "}" };
    case ',': return { TokenType::Comma, "," };
    }

    return { TokenType::None, "" };
}
