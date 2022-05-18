add_executable (RetransformTest RetransformTest.cpp)

target_link_libraries(RetransformTest Qt5::Test labplot2lib labplot2test)

# enable special access variables
target_compile_definitions(RetransformTest PRIVATE RETRANSFORMTEST_EN)
target_compile_definitions(labplot2lib PRIVATE RETRANSFORMTEST_EN)

add_test(NAME RetransformTest COMMAND RetransformTest)
