INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (FitTest FitTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(FitTest labplot2lib Qt5::Test)

add_test(NAME FitTest COMMAND FitTest)
