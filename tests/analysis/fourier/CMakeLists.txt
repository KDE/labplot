add_executable(FourierTest FourierTest.cpp)

target_link_libraries(FourierTest labplotbackendlib labplotlib labplottest)

add_test(NAME FourierTest COMMAND FourierTest)
