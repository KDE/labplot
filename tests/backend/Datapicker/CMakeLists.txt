add_executable (DatapickerTest DatapickerTest.cpp)

target_link_libraries(DatapickerTest Qt5::Test labplot2lib labplot2test)

add_test(NAME DatapickerTest COMMAND DatapickerTest)
