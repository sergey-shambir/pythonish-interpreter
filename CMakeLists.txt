project(pythonish-interpreter)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wpedantic -Wdocumentation")

aux_source_directory(src PROJECT_SRC)
add_executable(${PROJECT_NAME} ${PROJECT_SRC})
