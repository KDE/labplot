add_executable (NotebookTest NotebookTest.cpp)

target_link_libraries(NotebookTest labplot2lib labplot2test)

add_test(NAME NotebookTest COMMAND NotebookTest)
