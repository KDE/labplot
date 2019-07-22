INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslgeomtest NSLGeomTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslgeomtest Qt5::Test)
target_link_libraries(nslgeomtest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslgeomtest labplot2lib)

add_test(NAME nslgeomtest COMMAND nslgeomtest)
