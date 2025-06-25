add_executable(SmoothTest SmoothTest.cpp)

target_link_libraries(SmoothTest labplotbackendlib labplotlib labplottest)

add_test(NAME SmoothTest COMMAND SmoothTest)
