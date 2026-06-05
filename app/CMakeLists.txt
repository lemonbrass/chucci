file(GLOB_RECURSE SOURCE_FILES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(chuccic
  ${SOURCE_FILES}
)
target_link_libraries(chuccic PUBLIC
  chucci
)
target_include_directories(chuccic PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
