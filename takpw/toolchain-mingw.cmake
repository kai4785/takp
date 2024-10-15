SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_PROCESSOR i686)  # x86_64 Or i686

SET(CMAKE_C_COMPILER ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER ${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32-g++)

SET(CMAKE_FIND_ROOT_PATH /usr/${CMAKE_SYSTEM_PROCESSOR}-w64-mingw32)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
