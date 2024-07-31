add_executable(DatasetsTest DatasetsTest.cpp)

target_link_libraries(DatasetsTest labplot2backendlib labplot2lib labplot2test)

add_test(NAME DatasetsTest COMMAND DatasetsTest)
