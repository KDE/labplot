option(ENABLE_TEST_BACKEND_DATAPICKER "Enable Datapicker Tests" ON)

if(ENABLE_TEST_BACKEND_DATAPICKER)
    add_executable (DatapickerTest DatapickerTest.cpp)

    target_link_libraries(DatapickerTest labplot2backendlib labplot2lib labplot2test)

    add_test(NAME DatapickerTest COMMAND DatapickerTest)
endif()
