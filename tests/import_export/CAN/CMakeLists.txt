option(ENABLE_TEST_IMPORTEXPORT_CAN "Enable CAN Bus related Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT_CAN)
    add_executable (BLFFilterTest BLFFilterTest.cpp)

    target_link_libraries(BLFFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
    add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
endif()
