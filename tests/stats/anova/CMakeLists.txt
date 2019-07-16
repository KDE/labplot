add_executable (anova AnovaTest.cpp ../../CommonTest.cpp)

target_link_libraries(anova Qt5::Test)
target_link_libraries(anova KF5::Archive KF5::XmlGui)
target_link_libraries(anova labplot2lib)

add_test(NAME anova COMMAND anova)
