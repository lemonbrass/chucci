cmake_minimum_required(VERSION 3.20)
project(chucci C)
enable_testing()

add_subdirectory(libchucci)
add_subdirectory(cli)
add_subdirectory(test)
