cmake_minimum_required(VERSION 3.10)

option(ENABLE_PROFILING "Enable profiling" Off)
option(ENABLE_CODE_COVERAGE "Enable code coverage" Off)

enable_testing()

set(CMAKE_C_STANDARD 11)

project(eqdps VERSION 0.1.0)

if(WIN32)
else()
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-microsoft")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fms-extensions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-braces")
    message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
    if(ENABLE_PROFILING)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    endif()
    if(ENABLE_CODE_COVERAGE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    endif()
endif()

add_library(enchilada STATIC
    array.c
    array.h
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

add_executable(dps dps.c)
target_link_libraries(dps PRIVATE enchilada)

add_executable(dps-test test.c)
target_link_libraries(dps-test PRIVATE enchilada)
#add_test(dps-test dps-test)

set(UNITS
#    array
#    action
#    battle
#    config
#    date
#    tail
    utility
)
foreach(UNIT ${UNITS})
    add_executable(test-${UNIT} test-${UNIT}.c ${UNIT}.h test.h)
    target_link_libraries(test-${UNIT} PRIVATE enchilada)
    add_test(test-${UNIT} test-${UNIT})
endforeach()

install(TARGETS dps
    RUNTIME DESTINATION .
)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
