FILE(GLOB pngs "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
install(FILES ${pngs} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics )

FILE(GLOB svgs "${CMAKE_CURRENT_SOURCE_DIR}/*.svg")
install(FILES ${svgs} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics )

#kde4_update_iconcache()
