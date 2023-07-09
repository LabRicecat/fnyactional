cmake_minimum_required(VERSION 3.9)
project(meow-script 
    VERSION 0.0.1 
    LANGUAGES CXX
    DESCRIPTION "A pure functional programming language"
)
set(CMAKE_CXX_STANDARD 20)
set(BINARY fnya)
set(SOURCE 
    main.cpp
    )

add_executable(${BINARY} ${SOURCE})
