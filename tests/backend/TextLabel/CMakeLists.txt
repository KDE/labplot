add_executable (TextLabelTest TextLabelTest.cpp)

target_link_libraries(TextLabelTest Qt5::Test labplot2lib labplot2test)

add_test(NAME TextLabelTest COMMAND TextLabelTest)
