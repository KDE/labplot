INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (nslfittest NSLFitTest.cpp ../NSLTest.cpp ../../CommonTest.cpp)

target_link_libraries(nslfittest Qt5::Test)
target_link_libraries(nslfittest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslfittest labplot2lib)

add_test(NAME nslfittest COMMAND nslfittest)
