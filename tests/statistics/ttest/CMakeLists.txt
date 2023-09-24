add_executable (ttest TTestTest.cpp ../../CommonTest.cpp)
target_link_libraries(ttest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
add_test(NAME ttest COMMAND ttest)
