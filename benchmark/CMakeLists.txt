cmake_minimum_required(VERSION 3.26)
project(syexpr_benchmark CXX)

find_package(benchmark REQUIRED)

set(SRCS shunting_yard.benchmark.cpp evaluate.benchmark.cpp index.cpp)

include_directories(${benchmark_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} benchmark::benchmark_main syexpr)
