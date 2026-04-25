add_executable(DatasetsTest DatasetsTest.cpp)

target_link_libraries(DatasetsTest labplotbackendlib labplotlib labplottest)

add_test(NAME DatasetsTest COMMAND DatasetsTest)
