FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/LabPlot/pics )

# not working
#kde4_install_icons( ${DATA_INSTALL_DIR}/LabPlot/pics )
#kde4_install_icons( ${ICON_INSTALL_DIR} )
