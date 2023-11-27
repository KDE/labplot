add_executable (ColumnTest ColumnTest.cpp)

target_link_libraries(ColumnTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ColumnTest COMMAND ColumnTest)
