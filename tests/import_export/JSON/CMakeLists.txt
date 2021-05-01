add_executable (jsonfiltertest JsonFilterTest.cpp)

target_link_libraries(jsonfiltertest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(jsonfiltertest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
    target_link_libraries(jsonfiltertest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
    target_link_libraries(jsonfiltertest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
    target_link_libraries(jsonfiltertest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(jsonfiltertest ${HDF5_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
    target_link_libraries(jsonfiltertest liborigin-static)
ENDIF ()

target_link_libraries(jsonfiltertest labplot2lib)

add_test(NAME jsonfiltertest COMMAND jsonfiltertest)
