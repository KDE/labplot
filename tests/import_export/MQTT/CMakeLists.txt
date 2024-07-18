IF (Qt${QT_VERSION_MAJOR}Mqtt_FOUND)

	add_executable (MQTTTest MQTTTest.cpp)

	target_link_libraries(MQTTTest Qt${QT_VERSION_MAJOR}::Mqtt labplot2backendlib labplot2lib labplot2nsllib labplot2test)

	add_test(NAME MQTTTest COMMAND MQTTTest)

ENDIF ()
