option(ENABLE_TEST_IMPORTEXPORT_SPICE "Enable Spice Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT_SPICE)

add_executable (SpiceFilterTest SpiceFilterTest.cpp)
target_compile_definitions(SpiceFilterTest PRIVATE SPICEFILTERTEST_EN)

target_link_libraries(SpiceFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)
target_compile_definitions(labplot2backendlib PRIVATE SPICEFILTERTEST_EN)

add_test(NAME SpiceFilterTest COMMAND SpiceFilterTest)

endif()
