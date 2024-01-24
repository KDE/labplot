option(ENABLE_TEST_BACKEND_XYCURVE "Enable XYCurve Tests" ON)

if (ENABLE_TEST_BACKEND_XYCURVE)
    add_executable (XYCurveTest XYCurveTest.cpp)

    target_link_libraries(XYCurveTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME XYCurveTest COMMAND XYCurveTest)
endif()
