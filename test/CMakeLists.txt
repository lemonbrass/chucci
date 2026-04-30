file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_executable(all_tests ${TEST_SOURCES})
target_link_libraries(all_tests PRIVATE chucci_test)

target_compile_definitions(all_tests PRIVATE ASSERT_MODE_TEST)

add_test(NAME all_tests COMMAND all_tests)
