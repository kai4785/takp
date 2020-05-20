cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 17)

project(eqdps VERSION 0.4.0)

# tar --strip-components=1 -xvf ~/Downloads/asio-asio-1-16-1.tar.gz asio-asio-1-16-1/asio/include
add_library(asio INTERFACE)
target_include_directories(asio SYSTEM INTERFACE asio/include)
include(FindThreads)
target_link_libraries(asio INTERFACE Threads::Threads)

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

# Platform compiler flags
if(WIN32)
else()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-microsoft")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
    message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
    if(ENABLE_PROFILING)
        # NOTE: gprof dps gmon.out | vim -R -
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    endif()
    if(ENABLE_CODE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        add_custom_target(coverage_html
            lcov -c -d . -o lcov.info
            COMMAND genhtml lcov.info -o coverage_html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

# Static library with all the code
add_library(enchilada STATIC
    action.cpp
    action.h
    battle.cpp
    battle.h
    config.cpp
    config.h
    date.cpp
    date.h
    tail.cpp
    tail.h
    utility.cpp
    utility.h
)
target_link_libraries(enchilada PUBLIC asio)

# Main executable
add_executable(dps dps.cpp)
target_link_libraries(dps PRIVATE enchilada)
install(TARGETS dps RUNTIME DESTINATION .) 

# Here be testing
enable_testing()

set(UNITS
    action
    battle
#    config
    date
    tail
)
foreach(UNIT ${UNITS})
    add_executable(test-${UNIT} test-${UNIT}.cpp ${UNIT}.h test.h)
    target_link_libraries(test-${UNIT} PRIVATE enchilada)
    add_test(test-${UNIT} test-${UNIT})
endforeach()

# Make a distributable zip file
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
