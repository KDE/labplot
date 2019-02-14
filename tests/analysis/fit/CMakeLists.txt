INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (fittest FitTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(fittest Qt5::Test)
target_link_libraries(fittest KF5::Archive KF5::XmlGui ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(fittest labplot2lib)

add_test(NAME fittest COMMAND fittest)
