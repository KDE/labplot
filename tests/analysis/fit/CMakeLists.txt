add_executable(FitTest FitTest.cpp)

target_link_libraries(FitTest labplotbackendlib labplotlib labplottest)

add_test(NAME FitTest COMMAND FitTest)
