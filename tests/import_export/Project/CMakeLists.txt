add_executable (ProjectImportTest ProjectImportTest.cpp)

target_link_libraries(ProjectImportTest labplot2lib labplot2test)

add_test(NAME ProjectImportTest COMMAND ProjectImportTest)
