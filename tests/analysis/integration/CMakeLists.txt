INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (integrationtest IntegrationTest.cpp ../AnalysisTest.cpp)

target_link_libraries(integrationtest Qt5::Test)
target_link_libraries(integrationtest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(integrationtest labplot2lib)

add_test(NAME integrationtest COMMAND integrationtest)
