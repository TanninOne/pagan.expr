cmake_minimum_required(VERSION 3.26)
project(syexpr_benchmark CXX)

find_package(Catch2 CONFIG REQUIRED)

set(SRCS evaluate.benchmark.cpp shunting_yard.benchmark.cpp)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} Catch2::Catch2 Catch2::Catch2WithMain pagan::expr)
