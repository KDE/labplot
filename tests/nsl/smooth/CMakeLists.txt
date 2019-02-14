INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslsmoothtest NSLSmoothTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslsmoothtest Qt5::Test)
target_link_libraries(nslsmoothtest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslsmoothtest labplot2lib)

add_test(NAME nslsmoothtest COMMAND nslsmoothtest)
