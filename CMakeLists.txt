cmake_minimum_required(VERSION 4.0)
project(WeirdLang)

set(CMAKE_CXX_STANDARD 26)

add_executable(WeirdLang main.cpp
        src/Lexer.cpp
        include/Lexer.hpp
        src/Parser.cpp
        include/Parser.hpp
        include/AST/AST.hpp
        include/NativeFunctions.hpp
        include/AST/Value.hpp
        include/AST/Scope.hpp
        include/AST/Base.hpp)

target_include_directories(WeirdLang PUBLIC include)
