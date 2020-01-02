cmake_minimum_required(VERSION 3.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lm")

add_definitions(-Wall)
add_definitions(-Wextra)

add_custom_target(
    run
    COMMAND ./bin/koala examples/example.frs
)

project(koala)

add_subdirectory(src)

install(
    TARGETS koala
    DESTINATION /usr/bin/
)
