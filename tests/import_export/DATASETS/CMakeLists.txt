add_executable (datasettest DatasetsUnitTest.cpp)

target_link_libraries(datasettest Qt5::Test KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

IF (Qt5SerialPort_FOUND)
	target_link_libraries(datasettest Qt5::SerialPort)
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
	target_link_libraries(datasettest KF5::SyntaxHighlighting)
ENDIF ()
#TODO: KF5::NewStuff

IF (Cantor_FOUND)
	target_link_libraries(datasettest Cantor::cantorlibs)
ENDIF ()

target_link_libraries(datasettest ${HDF5_C_LIBRARIES} ${FFTW3_LIBRARIES} ${netCDF_LIBRARIES} ${CFITSIO_LIBRARIES} )

IF (USE_LIBORIGIN)
target_link_libraries(datasettest liborigin-static)
ENDIF ()

target_link_libraries(datasettest labplot2lib)

add_test(NAME datasettest COMMAND datasettest)
