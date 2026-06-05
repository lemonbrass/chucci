file(GLOB_RECURSE SOURCE_FILES 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(chucci
  ${SOURCE_FILES}
)

target_include_directories(chucci PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
