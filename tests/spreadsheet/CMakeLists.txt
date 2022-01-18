INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (SpreadsheetTest SpreadsheetTest.cpp)

target_link_libraries(SpreadsheetTest labplot2lib Qt5::Test)

add_test(NAME SpreadsheetTest COMMAND SpreadsheetTest)
