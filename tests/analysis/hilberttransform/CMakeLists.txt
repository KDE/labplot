add_executable (HilbertTransformTest HilbertTransformTest.cpp)

target_link_libraries(HilbertTransformTest labplot2lib Qt5::Test labplot2test)

add_test(NAME HilbertTransformTest COMMAND HilbertTransformTest)
