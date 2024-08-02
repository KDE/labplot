option(ENABLE_TEST_NOTEBOOK "Enable Notebook Tests" ON)

if(ENABLE_TEST_NOTEBOOK)
    add_executable (NotebookTest NotebookTest.cpp)

    target_link_libraries(NotebookTest labplotlib labplotbackendlib labplottest)

    add_test(NAME NotebookTest COMMAND NotebookTest)
endif()
