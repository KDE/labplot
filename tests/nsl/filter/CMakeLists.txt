add_executable(NSLFilterTest NSLFilterTest.cpp)

target_link_libraries(NSLFilterTest ${FFTW3_LIBRARIES} labplottest)

add_test(NAME NSLFilterTest COMMAND NSLFilterTest)
