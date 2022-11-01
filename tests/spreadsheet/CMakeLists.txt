add_executable (SpreadsheetTest SpreadsheetTest.cpp)

target_link_libraries(SpreadsheetTest labplot2lib Qt5::Test)

add_test(NAME SpreadsheetTest COMMAND SpreadsheetTest)

add_executable (FormulaTest FormulaTest.cpp)

target_link_libraries(FormulaTest labplot2lib Qt5::Test)

add_test(NAME FormulaTest COMMAND FormulaTest)
