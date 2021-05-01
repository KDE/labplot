INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslinttest NSLIntTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslinttest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
target_link_libraries(nslinttest labplot2lib)

add_test(NAME nslinttest COMMAND nslinttest)
