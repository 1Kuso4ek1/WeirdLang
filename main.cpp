#include "NativeFunctions.hpp"
#include "Parser.hpp"

#include <fstream>
#include <print>

std::string LoadCode(std::string&& path)
{
    std::ifstream file(path);
    if(!file.is_open())
        throw std::runtime_error("Failed to open file");

    std::string code;
    std::copy(
        std::istreambuf_iterator(file),
        std::istreambuf_iterator<char>(),
        std::back_inserter(code)
    );

    return code;
}

int main()
{
    Lexer lexer(LoadCode("../code.wrd"));
    Parser parser(lexer);

    DefineDefaultFunctions();

    if(const auto result = parser.GetRoot()->Evaluate(symbolTable))
        std::visit([](auto&& v) { std::println("Value: {}", v); }, *result);
}
