add_executable (ROOTFilterTest ROOTFilterTest.cpp)

target_link_libraries(ROOTFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ROOTFilterTest COMMAND ROOTFilterTest)
