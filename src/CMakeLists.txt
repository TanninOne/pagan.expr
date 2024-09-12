cmake_minimum_required(VERSION 3.26)
project(syexpr CXX)

set(HDRS shunting_yard.h token.h evaluate.h)
set(SRCS shunting_yard.cpp token.cpp evaluate.cpp)

add_library(${PROJECT_NAME} STATIC ${SRCS})

target_sources(${PROJECT_NAME}
  PUBLIC FILE_SET HEADERS FILES ${HDRS}
)

add_library(syexpr::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (MSVC)
    # stack guards have a fairly significant impact on performance
    target_compile_options(${PROJECT_NAME} PRIVATE /GS-)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

install(TARGETS syexpr
  CONFIGURATIONS Debug
  FILE_SET HEADERS
    DESTINATION include
  LIBRARY
    DESTINATION lib

)

install(TARGETS syexpr
  CONFIGURATIONS Release
  FILE_SET HEADERS
    DESTINATION include
  LIBRARY
    DESTINATION lib
)

