option(ENABLE_TEST_COMMONFRONTEND "Enable Common frontend Tests" ON)

if(ENABLE_TEST_COMMONFRONTEND)
    add_subdirectory(widgets)
endif()
