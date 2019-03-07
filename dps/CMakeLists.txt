cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_library(tail STATIC
    tail.c
    tail.h
)

add_executable(dps
    dps.c
)

target_link_libraries(dps
    PUBLIC
        tail
)
