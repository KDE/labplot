cmake_minimum_required(VERSION 3.16)

project(MqttTestPublisher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Widgets
    Mqtt
)

add_library(mqtt_test_publisher_lib STATIC
    BrownianMotionMqttClient.h
    BrownianMotionMqttClient.cpp)

target_link_libraries(mqtt_test_publisher_lib Qt6::Mqtt)

add_executable(${PROJECT_NAME}
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
    mqtt_test_publisher_lib
)

# Enable deprecated API warnings
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
)
