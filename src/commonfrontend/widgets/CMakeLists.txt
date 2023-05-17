find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS
    Gui
    Widgets
    Core
)


add_library(NumberSpinBox NumberSpinBox.cpp NumberSpinBox.h)
target_include_directories(NumberSpinBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_link_libraries(NumberSpinBox Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Core)
