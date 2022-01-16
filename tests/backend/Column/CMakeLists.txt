INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (ColumnTest ColumnTest.cpp ../../CommonTest.cpp)

target_link_libraries(ColumnTest Qt5::Test labplot2lib)

add_test(NAME ColumnTest COMMAND ColumnTest)
