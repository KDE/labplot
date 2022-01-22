INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (ProjectImportTest ProjectImportTest.cpp ../../CommonTest.cpp)

target_link_libraries(ProjectImportTest labplot2lib Qt5::Test)

add_test(NAME ProjectImportTest COMMAND ProjectImportTest)
