INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

add_executable (projectimporttest ProjectImportTest.cpp ../../CommonTest.cpp)

target_link_libraries(projectimporttest labplot2lib Qt5::Test)

add_test(NAME projectimporttest COMMAND projectimporttest)
