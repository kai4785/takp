cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

project(takp_launcher VERSION 0.0.1)

include(FindThreads)
set(launcher_SOURCES
    context.cpp
    context.h
    display.cpp
    display.h
    timestamp.cpp
    timestamp.h
    main.cpp
    window.cpp
    window.h
)

add_executable(takp_launcher ${launcher_SOURCES})
target_link_libraries(takp_launcher X11 Threads::Threads)

add_executable(takp_overlay overlay.cpp)
target_link_libraries(takp_overlay X11 Threads::Threads)
