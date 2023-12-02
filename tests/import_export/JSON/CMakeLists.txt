option(ENABLE_TEST_IMPORTEXPORT_JSON "Enable Json Tests" ON)
if (ENABLE_TEST_IMPORTEXPORT_JSON)

add_executable (JSONFilterTest JSONFilterTest.cpp)

target_link_libraries(JSONFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME JSONFilterTest COMMAND JSONFilterTest)

endif()
