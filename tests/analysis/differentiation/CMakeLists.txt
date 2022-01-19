INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (DifferentiationTest DifferentiationTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(DifferentiationTest labplot2lib Qt5::Test)

add_test(NAME DifferentiationTest COMMAND DifferentiationTest)
