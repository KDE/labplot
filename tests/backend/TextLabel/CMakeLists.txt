add_executable (TextLabelTest TextLabelTest.cpp)

target_link_libraries(TextLabelTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME TextLabelTest COMMAND TextLabelTest)
