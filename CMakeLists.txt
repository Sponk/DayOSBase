cmake_minimum_required(VERSION 2.8.0)

## FIXME: the DayOS gcc does not like -rdynamic, but CMake uses that
## when checking functionality.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -ldayos")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -ldayos -lcxxrt -lustl")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)

project(DayOSBase)

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
)

add_subdirectory(init)
add_subdirectory(shell)
add_subdirectory(tty)
add_subdirectory(vfs)
add_subdirectory(lua)
add_subdirectory(ramdisk)