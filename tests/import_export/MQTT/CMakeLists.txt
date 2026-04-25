add_subdirectory(MqttTestPublisher)

set(HOSTNAME "test.mosquitto.org")
set(PORT 1883)
set(PATHS 20)

add_executable (MqttPublisherNoGui MqttPublisherNoGui.cpp)
target_link_libraries(MqttPublisherNoGui Qt${QT_VERSION_MAJOR}::Core mqtt_test_publisher_lib)
set_property(TARGET MqttPublisherNoGui PROPERTY AUTOMOC ON)
target_compile_definitions(MqttPublisherNoGui PRIVATE HOSTNAME="${HOSTNAME}" PORT=${PORT} PATHS=${PATHS})


add_executable(MQTTTest MQTTTest.cpp)
target_link_libraries(MQTTTest Qt${QT_VERSION_MAJOR}::Mqtt labplotbackendlib labplotlib labplottest mqtt_test_publisher_lib)

add_test(NAME MQTTTest COMMAND MQTTTest)

target_compile_definitions(MQTTTest PRIVATE
    EXEC="${CMAKE_BINARY_DIR}/bin/MqttPublisherNoGui"
    HOSTNAME="${HOSTNAME}"
    PORT=${PORT}
    PATHS=${PATHS}
)
