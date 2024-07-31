option(ENABLE_TEST_MATRIX "Enable Matrix Tests" ON)

if(ENABLE_TEST_MATRIX)
    add_executable (MatrixTest MatrixTest.cpp)

    target_link_libraries(MatrixTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME MatrixTest COMMAND MatrixTest)

    add_executable (MatrixFormulaTest MatrixFormulaTest.cpp)

    target_link_libraries(MatrixFormulaTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME MatrixFormulaTest COMMAND MatrixFormulaTest)
endif()
