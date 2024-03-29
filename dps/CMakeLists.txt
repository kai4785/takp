cmake_minimum_required(VERSION 3.10)

project(eqdps VERSION 1.1.0)

option(ENABLE_PROFILING "Enable profiling" Off)
option(ENABLE_CODE_COVERAGE "Enable code coverage" Off)

# Platform include checks
set(CMAKE_INCLUDE_CURRENT_DIR On)
include(CheckIncludeFiles)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(io.h HAVE_IO_H)
check_include_files(windows.h HAVE_WINDOWS_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/system.h.in ${CMAKE_CURRENT_BINARY_DIR}/system.h)

message(STATUS "HAVE_UNISTD_H ${HAVE_UNISTD_H}")
message(STATUS "HAVE_IO_H ${HAVE_IO_H}")
message(STATUS "HAVE_WINDOWS_H ${HAVE_WINDOWS_H}")

set(CMAKE_C_STANDARD_REQUIRED On)
set(CMAKE_C_STANDARD 11)

# Platform compiler flags
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
        # NOTE: gprof dps gmon.out | vim -R -
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    endif()
    if(ENABLE_CODE_COVERAGE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        add_custom_target(coverage_html
            lcov -c -d . -o lcov.info
            COMMAND genhtml lcov.info -o coverage_html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

# Static library with all the code
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

# Main executable
add_executable(dps dps.c)
target_link_libraries(dps PRIVATE enchilada)
install(TARGETS dps RUNTIME DESTINATION .) 

# Here be testing
enable_testing()

set(UNITS
    array
    action
    battle
#    config
    date
    tail
    utility
)
foreach(UNIT ${UNITS})
    add_executable(test-${UNIT} test-${UNIT}.c ${UNIT}.h test.h)
    target_link_libraries(test-${UNIT} PRIVATE enchilada)
    add_test(test-${UNIT} test-${UNIT})
endforeach()

# Make a distributable zip file
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
