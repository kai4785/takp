cmake_minimum_required(VERSION 3.10)
project(takpw)

SET(CMAKE_STATIC_LIBRARY_PREFIX "")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")

SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -Wl,--enable-stdcall-fixup")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -Wl,--enable-stdcall-fixup")

add_library(takpw SHARED takpw.cpp version.rc d3d8.def)

add_executable(testd3d8 main.cpp)
target_link_libraries(testd3d8 PUBLIC d3d8)
