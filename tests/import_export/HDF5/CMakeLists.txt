add_executable (HDF5FilterTest HDF5FilterTest.cpp)

target_link_libraries(HDF5FilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME HDF5FilterTest COMMAND HDF5FilterTest)
