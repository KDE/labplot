add_executable(HilbertTransformTest HilbertTransformTest.cpp)

target_link_libraries(HilbertTransformTest labplotbackendlib labplotlib labplottest)

add_test(NAME HilbertTransformTest COMMAND HilbertTransformTest)
