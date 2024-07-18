option(ENABLE_TEST_IMPORTEXPORT_DATASET "Enable Dataset Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT_DATASET)

    add_executable (DatasetsTest DatasetsTest.cpp)

    # TODO: inherit from general test and remove labplot2backendlib, labplot2nsllib
    target_link_libraries(DatasetsTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

    add_test(NAME DatasetsTest COMMAND DatasetsTest)

endif()
