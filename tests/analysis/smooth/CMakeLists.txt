add_executable(SmoothTest SmoothTest.cpp)

target_link_libraries(SmoothTest labplotbackendlib labplotlib labplotmetatest)

add_test(NAME SmoothTest COMMAND SmoothTest)
