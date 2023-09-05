add_executable (DatapickerTest DatapickerTest.cpp)

target_link_libraries(DatapickerTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME DatapickerTest COMMAND DatapickerTest)
