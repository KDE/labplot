INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nsldfttest NSLDFTTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nsldfttest Qt5::Test ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${FFTW3_LIBRARIES})
target_link_libraries(nsldfttest labplot2lib)

add_test(NAME nsldfttest COMMAND nsldfttest)
