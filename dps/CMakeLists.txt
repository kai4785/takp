cmake_minimum_required(VERSION 3.10)

add_compile_options(-pg)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_C_STANDARD 11)

add_executable(dps
    action.c
    action.h
    dps.c
    config.c
    config.h
    date.c
    date.h
    tail.c
    tail.h
    utility.c
    utility.h
)
