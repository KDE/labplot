FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.pdf")
install(FILES ${files} DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/pics/gsl_distributions )
