#include "Parser.hpp"

#include <format>

#include "Lexer.hpp"
#include "NativeFunctions.hpp"

Parser::Parser(Lexer& lexer)
    : lexer(lexer)
{
    DeclareDefaultFunctions();

    NextToken();

    std::vector<ExprPtr> statements;

    while(currentToken.first != Lexer::TokenType::EndOfFile)
    {
        if(auto expr = Parse())
            statements.emplace_back(std::move(expr));
    }

    root = std::make_shared<StatementList>(std::move(statements));
    std::static_pointer_cast<StatementList>(root)->noLocalScope = true;
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
    case Lexer::TokenType::Char: return ParseChar();
    case Lexer::TokenType::Semicolon: NextToken(); break;

    case Lexer::TokenType::Arrow:
    case Lexer::TokenType::LeftBrace:
        return ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);

    case Lexer::TokenType::Bool:
    {
        auto expr = std::make_shared<ValueExpr>(currentToken.second == "true");

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

    default: return ParseUnary();
    }

    return nullptr;
}

ExprPtr Parser::ParseBinaryRight(const int leftPrec, ExprPtr left)
{
    if(currentToken.first == Lexer::TokenType::Increment
        || currentToken.first == Lexer::TokenType::Decrement)
    {
        auto expr = std::make_shared<UnaryExpr>(currentToken, std::move(left));
        expr->operationFirst = false;

        NextToken();

        left = std::move(expr);
    }

    while(true)
    {
        if(currentToken.first == Lexer::TokenType::LeftBracket)
        {
            NextToken();
            auto index = Parse();
            Expect(Lexer::TokenType::RightBracket);

            left = std::make_shared<IndexExpr>(std::move(left), std::move(index));

            continue;
        }

        const int currentPrec = GetPrecedence();

        if(currentPrec < leftPrec)
            return left;

        auto operation = currentToken;
        NextToken();

        auto right = ParsePrimary();

        if(const int nextPrec = GetPrecedence(); currentPrec < nextPrec)
            right = ParseBinaryRight(currentPrec + 1, std::move(right));

        left = std::make_shared<BinaryExpr>(
            operation, std::move(left), std::move(right)
        );
    }
}

ExprPtr Parser::ParseUnary()
{
    auto operation = currentToken;
    NextToken();

    auto expr = ParsePrimary();

    return std::make_shared<UnaryExpr>(operation, std::move(expr));
}

ExprPtr Parser::ParseReserved()
{
    const auto token = currentToken.second;

    if(token == "var" || token == "fun")
        return ParseVarOrFunc(token);
    if(token == "if")
        return ParseIf();
    if(token == "while")
        return ParseWhile();
    if(token == "for")
        return ParseFor();
    if(token == "return")
    {
        NextToken();
        return std::make_shared<ReturnExpr>(Parse());
    }
    if(token == "break")
    {
        NextToken();
        return std::make_shared<BreakExpr>();
    }
    if(token == "continue")
    {
        NextToken();
        return std::make_shared<ContinueExpr>();
    }
    if(token == "struct")
        return ParseStruct();

    return nullptr;
}

ExprPtr Parser::ParseIdentifier()
{
    auto name = currentToken.second;
    NextToken();

    if(currentToken.first == Lexer::TokenType::LeftParen)
    {
        NextToken();
        auto args = ParseArguments();

        if(globalScope->Contains(name))
        {
            auto node = globalScope->Get(name).get();
            if(dynamic_cast<StructDecl*>(node))
                return std::make_shared<ConstructorExpr>(std::move(name), std::move(args));
        }

        return std::make_shared<FunctionCall>(std::move(name), std::move(args));
    }

    return std::make_shared<VariableExpr>(name);
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

    return std::make_shared<ValueExpr>(value);
}

ExprPtr Parser::ParseString()
{
    const auto value = currentToken.second;
    NextToken();

    std::vector<Value> data;
    data.reserve(value.size() + 1);

    for(const char i : value)
        data.emplace_back(i);
    data.emplace_back('\0');

    dataSection.emplace_back(std::move(data));

    return std::make_shared<ValueExpr>(reinterpret_cast<size_t>(dataSection.back().data()));
}

ExprPtr Parser::ParseChar()
{
    auto value = currentToken.second;
    NextToken();

    return std::make_shared<ValueExpr>(value[1]);
}

ExprPtr Parser::ParseStatementList(const bool singleExpr)
{
    if(!singleExpr)
        NextToken();

    std::vector<ExprPtr> list;
    while(currentToken.first != Lexer::TokenType::RightBrace
        && currentToken.first != Lexer::TokenType::EndOfFile)
    {
        if(auto expr = Parse())
            list.emplace_back(std::move(expr));
        if(singleExpr)
            break;
    }

    if(!singleExpr)
        Expect(Lexer::TokenType::RightBrace);

    return std::make_shared<StatementList>(std::move(list));
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

ExprPtr Parser::ParseVarOrFunc(const std::string& token)
{
    NextToken();
    Expect(Lexer::TokenType::Identifier, false);

    const auto name = currentToken.second;
    NextToken();

    if(globalScope->Contains(name))
        throw std::runtime_error("Symbol already exists");

    if(token == "var")
    {
        //globalScope->Declare(name, std::make_shared<ValueExpr>(0));
        return std::make_shared<VariableDecl>(name, std::make_shared<ValueExpr>(Value(0)));
    }

    NextToken();

    auto args = ParseArguments();
    const auto list = ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);

    return std::make_shared<FunctionDecl>(name, std::make_shared<StatementList>(
        std::move(dynamic_cast<StatementList*>(list.get())->statements),
        std::move(args)
    ));
}

ExprPtr Parser::ParseIf()
{
    NextToken();
    Expect(Lexer::TokenType::LeftParen);

    auto condition = Parse();

    Expect(Lexer::TokenType::RightParen);

    auto then = ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);
    auto elseExpr = ExprPtr{};

    if(currentToken.second == "else")
    {
        NextToken();

        if(currentToken.second == "if")
            elseExpr = ParseIf();
        else
            elseExpr = ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);
    }

    return std::make_shared<IfStatement>(std::move(condition), std::move(then), std::move(elseExpr));
}

ExprPtr Parser::ParseWhile()
{
    NextToken();
    Expect(Lexer::TokenType::LeftParen);

    auto condition = Parse();

    Expect(Lexer::TokenType::RightParen);

    auto body = ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);

    return std::make_shared<WhileStatement>(std::move(condition), std::move(body));
}

ExprPtr Parser::ParseFor()
{
    NextToken();
    Expect(Lexer::TokenType::LeftParen);

    ExprPtr init{};
    if(currentToken.first != Lexer::TokenType::Semicolon)
        init = Parse();

    Expect(Lexer::TokenType::Semicolon);

    ExprPtr condition{};
    if(currentToken.first != Lexer::TokenType::Semicolon)
        condition = Parse();

    Expect(Lexer::TokenType::Semicolon);

    ExprPtr step{};
    if(currentToken.first != Lexer::TokenType::RightParen)
        step = Parse();

    Expect(Lexer::TokenType::RightParen);

    auto body = ParseStatementList(currentToken.first != Lexer::TokenType::LeftBrace);
    std::static_pointer_cast<StatementList>(body)->noLocalScope = true;

    return std::make_shared<ForStatement>(
        std::move(init), std::move(condition),
        std::move(step), std::move(body)
    );
}

ExprPtr Parser::ParseStruct()
{
    NextToken();
    const auto name = currentToken.second;
    NextToken();

    auto structDecl = std::make_shared<StructDecl>(name);

    Expect(Lexer::TokenType::LeftBrace);

    while(currentToken.first != Lexer::TokenType::RightBrace)
    {
        const auto token = currentToken.second;
        auto expr = ParseVarOrFunc(token);
        auto propertyName = std::static_pointer_cast<VariableDecl>(expr)->name;

        // Only variables are being ordered
        if(dynamic_cast<VariableDecl*>(expr.get()))
            structDecl->order.push_back(propertyName);

        structDecl->content[propertyName] = std::move(expr);
    }

    NextToken();

    globalScope->Declare(name, std::move(structDecl));

    return structDecl;
}

int Parser::GetPrecedence() const
{
    if(precedence.contains(currentToken.first))
        return precedence.at(currentToken.first);

    return -1;
}

void Parser::Expect(const Lexer::TokenType tokenType, const bool skip)
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
