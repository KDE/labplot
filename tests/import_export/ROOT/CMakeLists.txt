add_executable (ROOTFilterTest ROOTFilterTest.cpp)

target_link_libraries(ROOTFilterTest labplot2lib Qt5::Test labplot2test)

add_test(NAME ROOTFilterTest COMMAND ROOTFilterTest)
