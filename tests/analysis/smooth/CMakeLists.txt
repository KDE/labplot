add_executable(SmoothTest SmoothTest.cpp)

target_link_libraries(SmoothTest labplot2backendlib labplot2lib labplot2test)

add_test(NAME SmoothTest COMMAND SmoothTest)
