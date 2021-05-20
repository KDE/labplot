INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (matiofiltertest MatioFilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(matiofiltertest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(matiofiltertest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
    target_link_libraries(matiofiltertest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
    target_link_libraries(matiofiltertest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
    target_link_libraries(matiofiltertest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(matiofiltertest ${HDF5_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
    target_link_libraries(matiofiltertest liborigin-static)
ENDIF ()

target_link_libraries(matiofiltertest labplot2lib)

add_test(NAME matiofiltertest COMMAND matiofiltertest)
