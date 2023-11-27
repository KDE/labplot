add_executable (RetransformTest RetransformTest.cpp)

target_link_libraries(RetransformTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME RetransformTest COMMAND RetransformTest)
