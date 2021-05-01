INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslstatstest NSLStatsTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslstatstest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
target_link_libraries(nslstatstest labplot2lib)

add_test(NAME nslstatstest COMMAND nslstatstest)
