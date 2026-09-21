option(ENABLE_TEST_MATRIX "Enable Matrix Tests" ON)

if(ENABLE_TEST_MATRIX)
    add_executable (MatrixTest MatrixTest.cpp)
    target_link_libraries(MatrixTest labplotbackendlib labplotlib labplottest)
    add_test(NAME MatrixTest COMMAND MatrixTest)
endif()
