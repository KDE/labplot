add_executable (readstatfiltertest ReadStatFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(readstatfiltertest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(readstatfiltertest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
    target_link_libraries(readstatfiltertest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
    target_link_libraries(readstatfiltertest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
    target_link_libraries(readstatfiltertest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(readstatfiltertest ${HDF5_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
    target_link_libraries(readstatfiltertest liborigin-static)
ENDIF ()

target_link_libraries(readstatfiltertest labplot2lib)

add_test(NAME readstatfiltertest COMMAND readstatfiltertest)
