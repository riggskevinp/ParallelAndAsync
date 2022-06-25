cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(DataSharing_Examples LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    main.cpp
    DataSharing.hpp
    ../Hello/basics.hpp
)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.txt ${CMAKE_CURRENT_BINARY_DIR}/input.txt COPYONLY)

#Comment out to disable testing
#include(CTest)
#enable_testing()
#if(BUILD_TESTING)
#    add_subdirectory(tests)
#endif()