add_executable (DifferentiationTest DifferentiationTest.cpp)

target_link_libraries(DifferentiationTest labplot2lib Qt5::Test labplot2test)

add_test(NAME DifferentiationTest COMMAND DifferentiationTest)
