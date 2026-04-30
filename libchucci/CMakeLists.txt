file(GLOB_RECURSE DIHAH_SOURCES CONFIGURE_DEPENDS src/*.c)

add_library(chucci
    ${DIHAH_SOURCES}
)

add_library(chucci_test
    ${DIHAH_SOURCES}
)

target_compile_definitions(chucci_test PRIVATE ASSERT_MODE_TEST)
target_include_directories(chucci
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(chucci_test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
