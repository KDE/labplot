if(WIN32 OR APPLE)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${KDE_INSTALL_DATADIR} PATTERN "CMakeLists.txt" EXCLUDE)
else()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME} PATTERN "CMakeLists.txt" EXCLUDE)
endif()
