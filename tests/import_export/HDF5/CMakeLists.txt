add_executable (HDF5FilterTest HDF5FilterTest.cpp ../../CommonTest.cpp)

target_link_libraries(HDF5FilterTest labplot2lib Qt5::Test)

add_test(NAME HDF5FilterTest COMMAND HDF5FilterTest)
