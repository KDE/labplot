INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (integrationtest IntegrationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(integrationtest Qt5::Test)
target_link_libraries(integrationtest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
IF (APPLE)
	target_link_libraries(integrationtest KDMacTouchBar)
ENDIF ()

IF (ZLIB_FOUND)
    target_link_libraries(integrationtest labplot2lib ${ZLIB_LIBRARY})
ELSE ()
	target_link_libraries(integrationtest labplot2lib)
ENDIF()

add_test(NAME integrationtest COMMAND integrationtest)
