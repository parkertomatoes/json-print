cmake_minimum_required(VERSION 3.25)
project(json_print)

add_custom_target (single-source ALL
  COMMAND cpp-merge ${CMAKE_CURRENT_SOURCE_DIR}/src/json_print.hpp -o ${CMAKE_CURRENT_SOURCE_DIR}/json_print/json_print.hpp
)