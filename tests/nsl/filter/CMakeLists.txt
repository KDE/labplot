INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslfiltertest NSLFilterTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslfiltertest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES})
target_link_libraries(nslfiltertest labplot2lib)

add_test(NAME nslfiltertest COMMAND nslfiltertest)
