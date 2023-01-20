FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.xpm")
install(FILES ${files} DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME}/pics/colorchooser )
