add_executable(FourierTest FourierTest.cpp)

target_link_libraries(FourierTest labplotlib labplotbackendlib labplottest)

add_test(NAME FourierTest COMMAND FourierTest)
