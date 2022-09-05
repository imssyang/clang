cmake_minimum_required(VERSION 3.10)

project(CXXBasic VERSION 1.0)

option(USE_CLANG "build application with clang" ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(CXXBasic test.cpp)

