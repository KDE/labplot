add_executable(ROOTFilterTest ROOTFilterTest.cpp)

target_link_libraries(ROOTFilterTest labplotmetatest)

add_test(NAME ROOTFilterTest COMMAND ROOTFilterTest)
