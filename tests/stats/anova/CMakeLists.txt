add_executable (anova AnovaTest.cpp ../../CommonTest.cpp)
target_link_libraries(anova labplot2backendlib labplot2lib labplot2nsllib labplot2test)
add_test(NAME anova COMMAND anova)
