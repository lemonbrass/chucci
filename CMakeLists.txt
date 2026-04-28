cmake_minimum_required(VERSION 3.20)
project(chucci C)
enable_testing()

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O0")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

add_subdirectory(libchucci)
add_subdirectory(cli)
add_subdirectory(test)
