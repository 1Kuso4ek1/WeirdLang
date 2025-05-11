cmake_minimum_required(VERSION 4.0)
project(WeirdLang)

set(CMAKE_CXX_STANDARD 26)

add_executable(WeirdLang main.cpp
        src/Lexer.cpp
        include/Lexer.hpp
        src/Parser.cpp
        include/Parser.hpp
        include/AST.hpp
        include/NativeFunctions.hpp
        include/Value.hpp)

target_include_directories(WeirdLang PUBLIC include)
