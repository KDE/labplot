add_executable(HDF5FilterTest HDF5FilterTest.cpp)

target_link_libraries(HDF5FilterTest labplotmetatest)

add_test(NAME HDF5FilterTest COMMAND HDF5FilterTest)
