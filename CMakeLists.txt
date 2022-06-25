cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(ParallelAsync LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(Hello)
add_subdirectory(DataSharing)