add_executable(SerialPortTest SerialPortTest.cpp)
target_link_libraries(SerialPortTest Qt${QT_MAJOR_VERSION}::SerialPort labplotbackendlib labplotlib labplottest)

add_test(NAME SerialPortTest COMMAND SerialPortTest)
