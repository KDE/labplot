option(ENABLE_TEST_BACKEND_COLUMN "Enable Column Tests" ON)

if (ENABLE_TEST_BACKEND_COLUMN)

    add_executable (ColumnTest ColumnTest.cpp)

    target_link_libraries(ColumnTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME ColumnTest COMMAND ColumnTest)

endif()
