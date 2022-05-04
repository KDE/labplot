add_executable (SmoothTest SmoothTest.cpp)

target_link_libraries(SmoothTest labplot2lib Qt5::Test labplot2test)

add_test(NAME SmoothTest COMMAND SmoothTest)
