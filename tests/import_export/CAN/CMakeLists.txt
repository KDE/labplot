add_executable (BLFFilterTest BLFFilterTest.cpp)

target_link_libraries(BLFFilterTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME BLFFilterTest COMMAND BLFFilterTest)
