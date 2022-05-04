add_executable (ProjectImportTest ProjectImportTest.cpp)

target_link_libraries(ProjectImportTest labplot2lib Qt5::Test labplot2test)

add_test(NAME ProjectImportTest COMMAND ProjectImportTest)
