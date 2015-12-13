file(GLOB_RECURSE HEADERS *.h)
file(GLOB_RECURSE SOURCES *.c*)

add_executable(shell.elf ${SOURCES} ${HEADERS})

include_directories(${APPLICATION_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/tlli-master ${CMAKE_CURRENT_SOURCE_DIR}/tlli-master/include ${CMAKE_CURRENT_SOURCE_DIR}/tlli-master/src/util
${CMAKE_CURRENT_SOURCE_DIR}/tlli-master/src/include)
