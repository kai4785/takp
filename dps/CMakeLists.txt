cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_executable(dps
    dps.c
    config.c
    config.h
    date.c
    date.h
    tail.c
    tail.h
)
