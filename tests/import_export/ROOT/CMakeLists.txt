add_executable(ROOTFilterTest ROOTFilterTest.cpp)

target_link_libraries(ROOTFilterTest labplottest)

add_test(NAME ROOTFilterTest COMMAND ROOTFilterTest)
