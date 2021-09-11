cmake_minimum_required(VERSION 3.1.0)

project(example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MINIMUM_VERSION 5.6.0)
set(KF5_MIN_VERSION "5.16.0")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
	Config
	I18n
)

add_executable(example main.cpp)
INCLUDE_DIRECTORIES(/usr/include/labplot/)

target_link_libraries(example labplot Qt5::Widgets KF5::ConfigCore KF5::I18n)
