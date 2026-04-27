file(GLOB_RECURSE DIHAH_SOURCES CONFIGURE_DEPENDS src/*.c)

add_library(chucci
    ${DIHAH_SOURCES}
)

target_include_directories(chucci
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
