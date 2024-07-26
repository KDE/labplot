add_executable (LiveDataTest LiveDataTest.cpp)

target_link_libraries(LiveDataTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME LiveDataTest COMMAND LiveDataTest)
