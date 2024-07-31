option(ENABLE_TEST_BACKEND_WORKSHEET "Enable Worksheet Tests" ON)

if(ENABLE_TEST_BACKEND_WORKSHEET)
    add_executable (WorksheetElementTest WorksheetElementTest.cpp)

    target_link_libraries(WorksheetElementTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME WorksheetElementTest COMMAND WorksheetElementTest)

    add_executable (WorksheetTest WorksheetTest.cpp)

    target_link_libraries(WorksheetTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME WorksheetTest COMMAND WorksheetTest)
endif()
