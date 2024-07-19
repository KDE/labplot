option(ENABLE_TEST_BACKEND_RETRANSFORM "Enable Retransform Tests" ON)

if (ENABLE_TEST_BACKEND_RETRANSFORM)
	add_executable (RetransformTest RetransformTest.cpp)

    target_link_libraries(RetransformTest labplot2backendlib labplot2lib labplot2nsllib labplot2metatest)

    add_test(NAME RetransformTest COMMAND RetransformTest)
endif()
