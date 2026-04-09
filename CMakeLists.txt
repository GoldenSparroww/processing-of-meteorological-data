cmake_minimum_required(VERSION 3.18)

project(
        processing-of-meteorological-data
        VERSION 1.0
        DESCRIPTION "TODO"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE serial_src serial/main.cpp serial/src/*.cpp serial/include/*.h)
FILE(GLOB_RECURSE parallel_src parallel/main.cpp parallel/src/*.cpp parallel/include/*.h)

add_executable(serial.exe ${serial_src})
add_executable(parallel.exe ${parallel_src})

target_compile_options(serial.exe PRIVATE -Wall -Wextra)
target_compile_options(parallel.exe PRIVATE -Wall -Wextra)

