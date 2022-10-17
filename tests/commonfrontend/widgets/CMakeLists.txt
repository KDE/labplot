add_executable (WidgetsTest WidgetsTest.cpp)

target_link_libraries(WidgetsTest labplot2lib Qt5::Test labplot2test)

add_test(NAME WidgetsTest COMMAND WidgetsTest)

add_executable (NumberSpinBox NumberSpinBoxMain.cpp ../../../src/commonfrontend/widgets/NumberSpinBox.cpp)
set_property(TARGET NumberSpinBox PROPERTY AUTOMOC ON)
target_include_directories(NumberSpinBox PRIVATE ../../../src/commonfrontend/widgets)
target_link_libraries(NumberSpinBox Qt5::Core Qt5::Widgets KF5::ConfigCore)
