add_executable(SerialPortTest SerialPortTest.cpp)
target_link_libraries(SerialPortTest Qt${QT_VERSION_MAJOR}::SerialPort labplotbackendlib labplotlib labplottest mqtt_test_publisher_lib)

add_test(NAME SerialPortTest COMMAND SerialPortTest)
