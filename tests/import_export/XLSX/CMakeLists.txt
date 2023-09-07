add_executable (ExcelFilterTest ExcelFilterTest.cpp)

target_link_libraries(ExcelFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ExcelFilterTest COMMAND ExcelFilterTest)
