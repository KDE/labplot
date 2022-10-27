find_package(QT NAMES Qt5 NO_MODULE REQUIRED COMPONENTS
        Gui
        Widgets
        Core
)
find_package(Qt5 5.15.6 COMPONENTS
    Gui
    Widgets
    Core
)


add_library(NumberSpinBox NumberSpinBox.cpp NumberSpinBox.h)
target_include_directories(NumberSpinBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_link_libraries(NumberSpinBox Qt5::Gui Qt5::Widgets Qt5::Core)
