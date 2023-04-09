add_executable (SpreadsheetTest SpreadsheetTest.cpp)

target_link_libraries(SpreadsheetTest labplot2lib labplot2test)

add_test(NAME SpreadsheetTest COMMAND SpreadsheetTest)

add_executable (SpreadsheetFormulaTest SpreadsheetFormulaTest.cpp)

target_link_libraries(SpreadsheetFormulaTest labplot2lib labplot2test)

add_test(NAME SpreadsheetFormulaTest COMMAND SpreadsheetFormulaTest)
