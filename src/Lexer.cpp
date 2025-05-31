#include "Lexer.hpp"

#include <fstream>
#include <ranges>
#include <print>

Lexer::Lexer(const std::filesystem::path& path)
    : code(LoadCode(path))
{
    std::filesystem::current_path(path.parent_path());

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
        {
            tokens.emplace_back(ProcessIdentifier(i));
            if(tokens.back().second == "import")
            {
                importFilename = true;
                tokens.pop_back();
            }
        }
        else if(std::isdigit(*i))
            tokens.emplace_back(ProcessNumber(i));
        else if(*i == '"')
        {
            tokens.emplace_back(ProcessString(i));
            if(importFilename)
            {
                Lexer importLexer(tokens.back().second);

                tokens.reserve(tokens.size() + importLexer.tokens.size());
                tokens.insert(tokens.end(), importLexer.tokens.begin(), importLexer.tokens.end());

                importFilename = false;
                tokens.pop_back();
            }
        }
        else if(*i == '\'')
        {
            tokens.emplace_back(Token{ TokenType::Char, { 1, ProcessChar(++i) } });
            i += 1;
        }
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

Lexer::Token Lexer::ProcessNumber(StringIter& iter)
{
    auto value = ""s;

    bool floatingPoint{};

    while(std::isdigit(*iter)
        || (*iter == '.' && std::isdigit(*(iter + 1)))
        || (*iter == 'f' && !floatingPoint))
        value += *iter++;

    floatingPoint = (value.back() == 'f') && !floatingPoint;

    --iter;

    return { TokenType::Number, value };
}

Lexer::Token Lexer::ProcessString(StringIter& iter)
{
    auto value = ""s;

    while(*++iter != '"')
        value.append(1, ProcessChar(iter));

    return { TokenType::String, value };
}

Lexer::Token Lexer::ProcessOperator(const StringIter& iter)
{
    if(operatorTokensMap.contains(*iter))
    {
        auto currentOp = operatorTokensMap.at(*iter);
        if(tokens.empty())
            return currentOp;

        auto [type, string] = tokens.back();

        if(type != TokenType::String && string.size() == 1)
        {
            const auto p = std::pair{ string.at(0), currentOp.second.at(0) };

            if(doubleTokensMap.contains(p))
            {
                tokens.pop_back();
                return doubleTokensMap.at(p);
            }
        }

        return currentOp;
    }

    return { TokenType::None, "" };
}

std::string Lexer::LoadCode(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if(!file.is_open())
        throw std::runtime_error(std::format("Failed to open file {}. Current path: {}", path.string(), std::filesystem::current_path().string()));

    std::string code;
    std::copy(
        std::istreambuf_iterator(file),
        std::istreambuf_iterator<char>(),
        std::back_inserter(code)
    );

    return code;
}

char Lexer::ProcessChar(StringIter& iter)
{
    if(*iter == '\\')
    {
        switch(*++iter)
        {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'f': return '\f';
        case '0': return '\0';
        case '\'': return '\'';
        case '\"': return '\"';
        case '\\': return '\\';
        default: return *iter;
        }
    }

    return *iter;
}
