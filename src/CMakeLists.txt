cmake_minimum_required(VERSION 3.0)

include_directories(${PROJECT_SOURCE_DIR}/lib/std/)
include_directories(${PROJECT_SOURCE_DIR}/src/)

add_executable(
    koala
    main.c
    gnumber.c
    multisearcher.c
    interpreter.c
    parser.c
    util.c
    library.c
)
