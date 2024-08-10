add_executable(NSLFitTest NSLFitTest.cpp)

target_link_libraries(NSLFitTest ${LIBCERF_LIBRARIES} labplottest)

add_test(NAME NSLFitTest COMMAND NSLFitTest)
