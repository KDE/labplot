INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nsldifftest NSLDiffTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nsldifftest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
target_link_libraries(nsldifftest labplot2lib)

add_test(NAME nsldifftest COMMAND nsldifftest)
