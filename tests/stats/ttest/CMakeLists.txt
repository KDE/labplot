add_executable (ttest TTestTest.cpp ../../CommonTest.cpp)

target_link_libraries(ttest Qt5::Test)
target_link_libraries(ttest KF5::Archive KF5::XmlGui)
target_link_libraries(ttest labplot2lib)

add_test(NAME ttest COMMAND ttest)
