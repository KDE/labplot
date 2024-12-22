cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

project(example)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
set(QT_MAJOR_VERSION ${QT_VERSION_MAJOR})
set(QT_MIN_VERSION "6.5.0")
set(KF_MAJOR_VERSION "6")
set(KF_MIN_VERSION "5.240.0")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS Widgets REQUIRED)
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    I18n
)

add_executable(example main.cpp)
# portable?
include_directories(/usr/include/labplot/ ..)

# Be more relaxed with example code: remove definitions from directory definitions
#set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "")
# or at least remove QT_NO_CAST_FROM_ASCII
get_directory_property(directory_definitions COMPILE_DEFINITIONS)
list(REMOVE_ITEM directory_definitions "QT_NO_CAST_FROM_ASCII")
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${directory_definitions}")

# show directory definitions
#get_directory_property(DIRECTORY_DEFINITIONS COMPILE_DEFINITIONS)
#message(STATUS "EXAMPLE DIRECTORY DEFINITIONS = ${DIRECTORY_DEFINITIONS}\n")

if(ENABLE_SDK_EXAMPLES)	# in source build: link to liblabplot target
    target_link_libraries(example liblabplot Qt${QT_MAJOR_VERSION}::Widgets KF${KF_MAJOR_VERSION}::ConfigCore KF${KF_MAJOR_VERSION}::I18n)
else() # link to liblabplot lib
    target_link_libraries(example labplot Qt${QT_MAJOR_VERSION}::Widgets KF${KF_MAJOR_VERSION}::ConfigCore KF${KF_MAJOR_VERSION}::I18n)
endif()
