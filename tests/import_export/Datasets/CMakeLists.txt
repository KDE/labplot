add_executable(DatasetsTest DatasetsTest.cpp)

target_link_libraries(DatasetsTest labplotlib labplotbackendlib labplottest)

add_test(NAME DatasetsTest COMMAND DatasetsTest)
