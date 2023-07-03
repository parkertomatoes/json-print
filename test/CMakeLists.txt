cmake_minimum_required(VERSION 3.25)
project(JsonPrintTest)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/doctest)

# Test executable
add_executable(json_print_tests 
    main.cpp
    test_compile.cpp
    test_sprint.cpp
    test_errors.cpp
    test_print.cpp)
target_compile_features(json_print_tests PRIVATE cxx_std_17)
target_include_directories(json_print_tests INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest)
target_link_libraries(json_print_tests PRIVATE doctest::doctest)

# CTest
enable_testing()
include(${CMAKE_CURRENT_SOURCE_DIR}/doctest/scripts/cmake/doctest.cmake)
doctest_discover_tests(json_print_tests)