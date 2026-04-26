cmake_minimum_required(VERSION 3.18)

project(
        processing-of-meteorological-data
        VERSION 1.0
        DESCRIPTION "TODO"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE srcs main.cpp src/*.cpp include/*.h)

add_executable(upp_sp1.exe ${srcs})

target_compile_options(upp_sp1.exe PRIVATE -Wall -Wextra)

