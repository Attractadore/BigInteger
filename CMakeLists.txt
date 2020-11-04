cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_BUILD_TYPE DEBUG)

project(BigIntegerLib)

add_library(BigIntegerLib SHARED "src/BigInteger.cpp")

target_include_directories(BigIntegerLib PUBLIC
    "${PROJECT_SOURCE_DIR}/include/")
