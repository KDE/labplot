INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (CorrelationTest CorrelationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(CorrelationTest labplot2lib Qt5::Test)

add_test(NAME CorrelationTest COMMAND CorrelationTest)
