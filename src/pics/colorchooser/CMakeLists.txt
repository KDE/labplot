FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.xpm")
install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics/colorchooser )
