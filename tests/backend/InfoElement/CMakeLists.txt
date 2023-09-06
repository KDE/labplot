add_executable (InfoElementTest InfoElementTest.cpp)

target_link_libraries(InfoElementTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME InfoElementTest COMMAND InfoElementTest)
