option(ENABLE_TEST_BACKEND_XYCURVE "Enable XYCurve Tests" ON)

if(ENABLE_TEST_BACKEND_XYCURVE)
    add_executable (XYCurveTest XYCurveTest.cpp)

    target_link_libraries(XYCurveTest labplotbackendlib labplotlib labplottest)

    add_test(NAME XYCurveTest COMMAND XYCurveTest)
endif()

option(ENABLE_TEST_BACKEND_XYEQUATIONCURVE2 "Enable XYFunctionCurve Tests" ON)

if(ENABLE_TEST_BACKEND_XYEQUATIONCURVE2)
    add_executable (XYFunctionCurveTest XYFunctionCurveTest.cpp)

    target_link_libraries(XYFunctionCurveTest labplotbackendlib labplotlib labplottest)

    add_test(NAME XYFunctionCurveTest COMMAND XYFunctionCurveTest)
endif()
