INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (NSLGeomTest NSLGeomTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(NSLGeomTest labplot2lib Qt5::Test)

add_test(NAME NSLGeomTest COMMAND NSLGeomTest)
