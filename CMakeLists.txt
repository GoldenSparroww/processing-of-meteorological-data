cmake_minimum_required(VERSION 3.18)

project(
        processing-of-meteorological-data
        VERSION 1.0
        DESCRIPTION "TODO"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        main.cpp
        src/data_loader.cpp
        src/data_processor.cpp
        src/output_generator.cpp
)

add_executable(upp_sp1.exe ${SOURCES})

target_include_directories(upp_sp1.exe PRIVATE include)

target_compile_options(upp_sp1.exe PRIVATE -Wall -Wextra)

