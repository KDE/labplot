IF (APPLE)
	add_subdirectory(kdmactouchbar)
ENDIF()

# preview.sty
install(FILES preview.sty DESTINATION  ${DATA_INSTALL_DIR}/${PROJECT_NAME}/latex)

