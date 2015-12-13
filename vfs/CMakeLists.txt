file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.c*)

set(CMAKE_C_FLAGS "-nostdlib")
add_executable(vfs.elf ${SOURCES} ${HEADERS})
