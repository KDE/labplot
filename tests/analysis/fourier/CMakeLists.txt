add_executable (FourierTest FourierTest.cpp)

target_link_libraries(FourierTest labplot2lib Qt5::Test labplot2test)

add_test(NAME FourierTest COMMAND FourierTest)
