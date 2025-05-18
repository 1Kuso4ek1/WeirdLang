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

    const auto programScope = std::make_shared<Scope>(globalScope);

    parser.GetRoot()->Evaluate(programScope);

    if(const auto result = programScope->Get("main")->Evaluate(programScope))
        std::visit([](auto&& v)
        {
            if constexpr (!std::is_same_v<std::decay_t<decltype(v)>, std::any>)
                std::println("Value: {}", v);
        }, *result);
}
