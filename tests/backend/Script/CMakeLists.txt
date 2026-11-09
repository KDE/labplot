option(ENABLE_TEST_BACKEND_SCRIPT "Enable Script Tests" ON)

if(ENABLE_TEST_BACKEND_SCRIPT AND ENABLE_PYTHON_SCRIPTING)
    add_executable (ScriptTest ScriptTest.cpp)

    target_link_libraries(ScriptTest labplotbackendlib labplotlib labplotmetatest)

    add_test(NAME ScriptTest COMMAND ScriptTest)
endif()
