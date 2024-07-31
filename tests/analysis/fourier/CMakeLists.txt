add_executable(FourierTest FourierTest.cpp)

target_link_libraries(FourierTest labplot2backendlib labplot2lib labplot2test)

add_test(NAME FourierTest COMMAND FourierTest)
