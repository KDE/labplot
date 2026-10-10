option(ENABLE_TEST_BACKEND_RETRANSFORM "Enable Retransform Tests" ON)

if(ENABLE_TEST_BACKEND_RETRANSFORM)
    add_executable (RetransformTest RetransformTest.cpp)

    target_link_libraries(RetransformTest labplottest)

    add_test(NAME RetransformTest COMMAND RetransformTest)
endif()
