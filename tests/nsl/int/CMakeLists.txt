INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslinttest NSLIntTest.cpp)

target_link_libraries(nslinttest Qt5::Test)
target_link_libraries(nslinttest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslinttest labplot2lib)

add_test(NAME nslinttest COMMAND nslinttest)
