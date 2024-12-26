add_executable(LiveDataTest LiveDataTest.cpp)

set(HOSTNAME "127.0.0.1")
set(UDP_PORT 1027)
set(TCP_PORT 1234)
set(PUBLISH_TIME_MS 10)

# Helper application sending tcp/udp packages
add_executable (UdpTcpPublisher UdpTcpPublisher.cpp)
target_link_libraries(UdpTcpPublisher Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)
set_property(TARGET UdpTcpPublisher PROPERTY AUTOMOC ON)
target_compile_definitions(UdpTcpPublisher PRIVATE HOSTNAME="${HOSTNAME}" UDP_PORT=${UDP_PORT} TCP_PORT=${TCP_PORT} PUBLISH_TIME_MS=${PUBLISH_TIME_MS})


target_link_libraries(LiveDataTest labplotbackendlib labplotlib labplottest)

add_test(NAME LiveDataTest COMMAND LiveDataTest)

target_compile_definitions(LiveDataTest PRIVATE
    EXEC="${CMAKE_BINARY_DIR}/bin/UdpTcpPublisher"
    HOSTNAME="${HOSTNAME}"
    UDP_PORT=${UDP_PORT}
    TCP_PORT=${TCP_PORT}
    PUBLISH_TIME_MS=${PUBLISH_TIME_MS}
)
