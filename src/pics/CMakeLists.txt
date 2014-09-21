FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics )

FILE(GLOB svgs "${CMAKE_CURRENT_SOURCE_DIR}/*.svg")
install(FILES ${svgs} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics )

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/gsl_distributions/*.jpg")
install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics/gsl_distributions )
