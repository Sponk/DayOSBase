file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.c*)

add_executable(tty.drv ${SOURCES} ${HEADERS})
