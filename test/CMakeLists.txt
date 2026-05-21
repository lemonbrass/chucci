file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_executable(all_tests ${TEST_SOURCES})
target_link_libraries(all_tests PRIVATE chucci)

add_test(NAME all_tests COMMAND all_tests)
