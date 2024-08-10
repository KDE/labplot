add_executable(NSLGeomTest NSLGeomTest.cpp)

target_link_libraries(NSLGeomTest labplottest)

add_test(NAME NSLGeomTest COMMAND NSLGeomTest)
