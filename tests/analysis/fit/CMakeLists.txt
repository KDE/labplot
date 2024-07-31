add_executable(FitTest FitTest.cpp)

target_link_libraries(FitTest labplot2backendlib labplot2lib labplot2test)

add_test(NAME FitTest COMMAND FitTest)
