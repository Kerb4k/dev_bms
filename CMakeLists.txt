cmake_minimum_required(VERSION 3.20)

project("dev_bms" C CXX ASM)

include(cmake/st-project.cmake)
#include heade files and source files from Core/Inc and Core/Src
file(GLOB_RECURSE USER_INC ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/*.h)
file(GLOB_RECURSE USER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c)

add_executable(${PROJECT_NAME})
add_st_target_properties(${PROJECT_NAME})