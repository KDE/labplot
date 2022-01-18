IF (Qt5Mqtt_FOUND)

add_executable (MQTTUnitTest MQTTUnitTest.cpp)

target_link_libraries(MQTTUnitTest Qt5::Mqtt labplot2lib Qt5::Test)

add_test(NAME MQTTUnitTest COMMAND MQTTUnitTest)

ENDIF ()
