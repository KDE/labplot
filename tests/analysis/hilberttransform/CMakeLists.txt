INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (HilbertTransformTest HilbertTransformTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(HilbertTransformTest labplot2lib Qt5::Test)

add_test(NAME HilbertTransformTest COMMAND HilbertTransformTest)
