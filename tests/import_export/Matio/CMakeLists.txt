add_executable (MatioFilterTest MatioFilterTest.cpp)

target_link_libraries(MatioFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME MatioFilterTest COMMAND MatioFilterTest)
