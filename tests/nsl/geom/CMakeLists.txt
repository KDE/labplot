add_executable (NSLGeomTest NSLGeomTest.cpp)

target_link_libraries(NSLGeomTest labplot2lib labplot2test)

add_test(NAME NSLGeomTest COMMAND NSLGeomTest)
