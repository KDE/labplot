INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NotebookTest NotebookTest.cpp)

target_link_libraries(NotebookTest labplot2lib Qt5::Test)

add_test(NAME NotebookTest COMMAND NotebookTest)
