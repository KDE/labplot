INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (TextLabelTest TextLabelTest.cpp ../../CommonTest.cpp)

target_link_libraries(TextLabelTest Qt5::Test labplot2lib)

add_test(NAME TextLabelTest COMMAND TextLabelTest)
