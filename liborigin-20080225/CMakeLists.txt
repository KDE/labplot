add_library (labplot_origin OPJFile.cpp)
set_target_properties(labplot_origin PROPERTIES VERSION 0.0.4 SOVERSION 0)

#install(TARGETS origin DESTINATION lib)

add_executable(opj2dat opj2dat.cpp)
target_link_libraries (opj2dat labplot_origin)

install(TARGETS opj2dat DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS labplot_origin  DESTINATION ${LIB_INSTALL_DIR})

