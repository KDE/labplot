INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslfiltertest NSLFilterTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslfiltertest Qt5::Test)
target_link_libraries(nslfiltertest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

# TODO: link also with FFTW when available
# -DHAVE_FFTW3 -lfftw3 -lgsl -lgslcblas

target_link_libraries(nslfiltertest labplot2lib)

add_test(NAME nslfiltertest COMMAND nslfiltertest)
