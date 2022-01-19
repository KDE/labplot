INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (IntegrationTest IntegrationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(IntegrationTest labplot2lib Qt5::Test)

add_test(NAME IntegrationTest COMMAND IntegrationTest)
