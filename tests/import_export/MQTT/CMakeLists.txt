IF (Qt5Mqtt_FOUND)

add_executable (MQTTUnitTest MQTTUnitTest.cpp)

target_link_libraries(MQTTUnitTest Qt5::Mqtt labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME MQTTUnitTest COMMAND MQTTUnitTest)

ENDIF ()
