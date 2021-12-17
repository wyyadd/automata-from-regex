cmake_minimum_required(VERSION 3.20)
project(automata-from-regex)

set(CMAKE_CXX_STANDARD 20)

add_executable(automata-from-regex main.cpp Re2NFA.cpp Re2NFA.h NFA2DFA.cpp NFA2DFA.h DFA2mDFA.cpp DFA2mDFA.h)
