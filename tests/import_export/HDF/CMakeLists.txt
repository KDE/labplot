INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (hdf5filtertest HDF5FilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(hdf5filtertest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(hdf5filtertest KDMacTouchBar)
ENDIF ()

IF (Qt5SerialPort_FOUND)
    target_link_libraries(hdf5filtertest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
    target_link_libraries(hdf5filtertest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
    target_link_libraries(hdf5filtertest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(hdf5filtertest ${HDF5_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES})

IF (USE_LIBORIGIN)
    target_link_libraries(hdf5filtertest liborigin-static)
ENDIF ()

target_link_libraries(hdf5filtertest labplot2lib)

add_test(NAME hdf5filtertest COMMAND hdf5filtertest)
