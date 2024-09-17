set(HDRS shunting_yard.h token.h evaluate.h)
set(SRCS shunting_yard.cpp token.cpp evaluate.cpp)

add_library(${PROJECT_NAME} STATIC ${SRCS})

add_library(pagan::expr ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PUBLIC FILE_SET HEADERS FILES ${HDRS}
)

if (MSVC)
    # stack guards have a fairly significant impact on performance
    target_compile_options(${PROJECT_NAME} PRIVATE /GS-)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME "pagan::expr")
