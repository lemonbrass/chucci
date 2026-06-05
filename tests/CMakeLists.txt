file(GLOB_RECURSE SOURCE_FILES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(unit_tests
  ${SOURCE_FILES}
)

target_link_libraries(unit_tests PUBLIC
  chucci
)
