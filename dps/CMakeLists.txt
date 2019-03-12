cmake_minimum_required(VERSION 3.10)

add_compile_options(-pg)
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_C_STANDARD 11)

add_library(enchilada STATIC
    action.c
    action.h
    battle.c
    battle.h
    config.c
    config.h
    date.c
    date.h
    tail.c
    tail.h
    utility.c
    utility.h
)

add_executable(dps
    dps.c
)

target_link_libraries(dps
    PRIVATE
        enchilada
)

add_executable(dps-test
    test.c
)

target_link_libraries(dps-test
    PRIVATE
        enchilada
)

