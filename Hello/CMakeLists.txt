cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Hello LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(hello
    main.cpp
        basics.h
)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.txt ${CMAKE_CURRENT_BINARY_DIR}/input.txt COPYONLY)

#Comment out to disable testing
#include(CTest)
#enable_testing()
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()