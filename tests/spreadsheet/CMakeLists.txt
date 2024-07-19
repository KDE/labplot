option(ENABLE_TEST_SPREADSHEET "Enable Spreadsheet Tests" ON)

if (ENABLE_TEST_SPREADSHEET)

    add_executable (SpreadsheetTest SpreadsheetTest.cpp)
    target_link_libraries(SpreadsheetTest labplot2backendlib labplot2lib labplot2nsllib labplot2metatest)
    add_test(NAME SpreadsheetTest COMMAND SpreadsheetTest)

    add_executable (SpreadsheetFormulaTest SpreadsheetFormulaTest.cpp)
    target_link_libraries(SpreadsheetFormulaTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
    add_test(NAME SpreadsheetFormulaTest COMMAND SpreadsheetFormulaTest)

    add_executable (SpreadsheetGenerateDataTest SpreadsheetGenerateDataTest.cpp)
    target_link_libraries(SpreadsheetGenerateDataTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
    add_test(NAME SpreadsheetGenerateDataTest COMMAND SpreadsheetGenerateDataTest)

endif()
