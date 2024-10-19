cmake_minimum_required(VERSION 3.10)
project(takpw)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -Wl,--enable-stdcall-fixup")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -Wl,--enable-stdcall-fixup")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

#add_library(takpw SHARED takpw.cpp d3d8main.c version.rc d3d8.def)
add_library(takpw SHARED takpw.c d3d8main.c version.rc d3d8.def)

add_executable(testd3d8 main.cpp)
target_link_libraries(testd3d8 PUBLIC d3d8)
