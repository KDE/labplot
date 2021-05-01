add_executable (projectimporttest ProjectImportTest.cpp)

target_link_libraries(projectimporttest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(projectimporttest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
	target_link_libraries(projectimporttest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
	target_link_libraries(projectimporttest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
	target_link_libraries(projectimporttest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(projectimporttest ${HDF5_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
target_link_libraries(projectimporttest liborigin-static)
ENDIF ()

target_link_libraries(projectimporttest labplot2lib)

add_test(NAME projectimporttest COMMAND projectimporttest)
