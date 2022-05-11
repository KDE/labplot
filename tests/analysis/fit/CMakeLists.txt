add_executable (FitTest FitTest.cpp)

target_link_libraries(FitTest labplot2lib Qt5::Test labplot2test)

add_test(NAME FitTest COMMAND FitTest)
