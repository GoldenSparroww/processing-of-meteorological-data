cmake_minimum_required(VERSION 3.18)

project(
        processing-of-meteorological-data
        VERSION 1.0
        DESCRIPTION "Meteorological Data Processing with OpenMP"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

IF(MSVC)
    ADD_COMPILE_OPTIONS("/openmp")
    ADD_LINK_OPTIONS("/openmp")
ELSE()
    ADD_COMPILE_OPTIONS("-fopenmp")
    ADD_LINK_OPTIONS("-fopenmp")
    # MinGW
    LINK_LIBRARIES(gomp)
ENDIF()

set(SOURCES
        main.cpp
        src/data_loader.cpp
        src/data_processor.cpp
        src/output_generator.cpp
)

add_executable(upp_sp1 ${SOURCES})

target_include_directories(upp_sp1 PRIVATE include)

target_compile_options(upp_sp1 PRIVATE -Wall -Wextra)

