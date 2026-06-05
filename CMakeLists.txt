cmake_minimum_required(VERSION 3.15)
project(chucci C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libchucci)
add_subdirectory(app)
add_subdirectory(tests)
add_subdirectory(repl)


execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
)
