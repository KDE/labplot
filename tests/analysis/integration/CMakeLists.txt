add_executable (IntegrationTest IntegrationTest.cpp)

target_link_libraries(IntegrationTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME IntegrationTest COMMAND IntegrationTest)
