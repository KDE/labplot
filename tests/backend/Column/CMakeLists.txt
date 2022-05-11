add_executable (ColumnTest ColumnTest.cpp)

target_link_libraries(ColumnTest Qt5::Test labplot2lib labplot2test)

add_test(NAME ColumnTest COMMAND ColumnTest)
