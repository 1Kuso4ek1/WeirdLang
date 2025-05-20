#include "NativeFunctions.hpp"
#include "Parser.hpp"

#include <fstream>
#include <print>

int main(int argc, char** argv)
{
    if(argc < 2)
        throw std::runtime_error("You should specify the filename");

    Lexer lexer({ argv[1] });
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
