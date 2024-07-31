add_executable(NSLGeomTest NSLGeomTest.cpp)

target_link_libraries(NSLGeomTest labplot2nsllib ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} labplot2test)

add_test(NAME NSLGeomTest COMMAND NSLGeomTest)
