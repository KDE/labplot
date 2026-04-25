add_executable(SmoothTest SmoothTest.cpp)

target_link_libraries(SmoothTest labplotlib labplotbackendlib labplottest)

add_test(NAME SmoothTest COMMAND SmoothTest)
