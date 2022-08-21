cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(UnitTesting LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
find_library(DataSharing NAMES DataSharing)
include_directories(..)

add_executable(${CMAKE_PROJECT_NAME}
    UnitTests.cpp
)

gtest_discover_tests(Test)

target_link_libraries(Test
    gtest
    gtest_main
    DataSharing
)

