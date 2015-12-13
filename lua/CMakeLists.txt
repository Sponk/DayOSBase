file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE SOURCES src/*.c*)

add_executable(lua.elf ${SOURCES} ${HEADERS})
