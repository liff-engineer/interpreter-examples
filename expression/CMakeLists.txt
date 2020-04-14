cmake_minimum_required(VERSION 3.15)

project(expression
    LANGUAGES CXX 
)


add_executable(parserDemo)
target_sources(parserDemo
    PRIVATE parserDemo.cpp
)

add_executable(parseAsAST)
target_sources(parseAsAST
    PRIVATE parseAsAST.cpp
)
