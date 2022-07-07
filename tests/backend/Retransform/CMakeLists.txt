add_executable (RetransformTest RetransformTest.cpp)

target_link_libraries(RetransformTest Qt5::Test labplot2lib labplot2test)

add_test(NAME RetransformTest COMMAND RetransformTest)
