add_executable (asciifiltertest AsciiFilterTest.cpp)

target_link_libraries(asciifiltertest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(asciifiltertest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
	target_link_libraries(asciifiltertest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
	target_link_libraries(asciifiltertest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
	target_link_libraries(asciifiltertest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(asciifiltertest ${HDF5_C_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
	target_link_libraries(asciifiltertest liborigin-static)
ENDIF ()

target_link_libraries(asciifiltertest labplot2lib)

add_test(NAME asciifiltertest COMMAND asciifiltertest)
