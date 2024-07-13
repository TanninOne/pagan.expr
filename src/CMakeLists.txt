cmake_minimum_required(VERSION 3.26)
project(syexpr CXX)

set(HDRS shunting_yard.h token.h evaluate.h)
set(SRCS shunting_yard.cpp token.cpp evaluate.cpp)

add_library(${PROJECT_NAME} STATIC ${HDRS} ${SRCS})

if (MSVC)
    # stack guards have a fairly significant impact on performance
    target_compile_options(${PROJECT_NAME} PRIVATE /GS-)
endif()

# set_target_properties(${PROJECT_NAME} PROPERTIES CXX)
# set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
