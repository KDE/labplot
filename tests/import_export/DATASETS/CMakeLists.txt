add_executable (DatasetsTest DatasetsTest.cpp)

target_link_libraries(DatasetsTest labplot2lib Qt5::Test)

add_test(NAME DatasetsTest COMMAND DatasetsTest)
