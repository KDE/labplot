cmake_minimum_required(VERSION 3.16)
project(numberSpinBoxPlugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/plugins/designer")

find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Core Gui UiPlugin Widgets)

qt_add_plugin(NumberSpinBoxPlugin)
target_sources(NumberSpinBoxPlugin PRIVATE
    NumberSpinBoxPlugin.cpp NumberSpinBoxPlugin.h
)

add_subdirectory(../../commonfrontend/widgets widgets)

set_target_properties(NumberSpinBoxPlugin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(NumberSpinBoxPlugin PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiPlugin
    Qt::Widgets
    NumberSpinBox
)

install(TARGETS NumberSpinBoxPlugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

