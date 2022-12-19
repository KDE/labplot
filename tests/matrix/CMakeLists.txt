add_executable (MatrixTest MatrixTest.cpp)

target_link_libraries(MatrixTest labplot2lib Qt5::Test labplot2test)

add_test(NAME MatrixTest COMMAND MatrixTest)

add_executable (MatrixFormulaTest MatrixFormulaTest.cpp)

target_link_libraries(MatrixFormulaTest labplot2lib Qt5::Test labplot2test)

add_test(NAME MatrixFormulaTest COMMAND MatrixFormulaTest)
