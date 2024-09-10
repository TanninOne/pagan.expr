cmake_minimum_required(VERSION 3.26)
project(parser_tests CXX)

enable_testing()

set(SRCS shunting_yard.test.cpp evaluate.test.cpp integration.test.cpp)

find_package(Catch2 CONFIG REQUIRED)

include_directories(${GTest_INCLUDE_DIRS})
include(GoogleTest)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} Catch2::Catch2 Catch2::Catch2WithMain syexpr::syexpr)

add_compile_options("/std:c++latest")
add_test(NAME ${PROJECT_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
