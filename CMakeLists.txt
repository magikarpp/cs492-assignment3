cmake_minimum_required(VERSION 3.7)
project(filesystem)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c shell.c DTree.c LinkedList.c LDisk.c Lfile.c Queue.c datatypes.h)
add_executable(filesystem ${SOURCE_FILES})
target_link_libraries(filesystem m)
