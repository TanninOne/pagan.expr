cmake_minimum_required(VERSION 3.26)
project(syexpr CXX)

set(HDRS shunting_yard.h token.h evaluate.h)
set(SRCS shunting_yard.cpp token.cpp evaluate.cpp)

add_library(${PROJECT_NAME} STATIC ${HDRS} ${SRCS})
add_library(syexpr::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (MSVC)
    # stack guards have a fairly significant impact on performance
    target_compile_options(${PROJECT_NAME} PRIVATE /GS-)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_sources(${PROJECT_NAME} INTERFACE FILE_SET HEADERS FILES ${HDRS})
