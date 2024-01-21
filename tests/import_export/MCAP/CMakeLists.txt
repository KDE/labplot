option(ENABLE_TEST_IMPORTEXPORT_MCAP "Enable MCAP Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT_MCAP)

add_executable (MCAPFilterTest MCAPFilterTest.cpp)

target_link_libraries(MCAPFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME MCAPFilterTest COMMAND MCAPFilterTest)

endif()
