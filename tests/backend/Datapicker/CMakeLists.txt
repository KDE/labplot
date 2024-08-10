option(ENABLE_TEST_BACKEND_DATAPICKER "Enable Datapicker Tests" ON)

if(ENABLE_TEST_BACKEND_DATAPICKER)
    add_executable (DatapickerTest DatapickerTest.cpp)

    target_link_libraries(DatapickerTest labplotbackendlib labplotlib labplottest)

    add_test(NAME DatapickerTest COMMAND DatapickerTest)
endif()
