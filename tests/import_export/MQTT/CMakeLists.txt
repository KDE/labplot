add_executable(MQTTTest MQTTTest.cpp)

target_link_libraries(MQTTTest Qt${QT_VERSION_MAJOR}::Mqtt labplotbackendlib labplotlib labplottest)

add_test(NAME MQTTTest COMMAND MQTTTest)
