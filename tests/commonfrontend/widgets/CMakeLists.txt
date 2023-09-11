add_executable (WidgetsTest WidgetsTest.cpp)

target_link_libraries(WidgetsTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME WidgetsTest COMMAND WidgetsTest)

add_executable (NumberSpinBox NumberSpinBoxMain.cpp ../../../src/commonfrontend/widgets/NumberSpinBox.cpp)
set_property(TARGET NumberSpinBox PROPERTY AUTOMOC ON)
target_include_directories(NumberSpinBox PRIVATE ../../../src/commonfrontend/widgets)
target_link_libraries(NumberSpinBox Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Widgets KF${KF_MAJOR_VERSION}::ConfigCore KF${KF_MAJOR_VERSION}::I18n)
