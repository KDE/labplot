INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})
add_executable (SmoothTest SmoothTest.cpp ../AnalysisTest.cpp ../../CommonTest.cpp)

target_link_libraries(SmoothTest labplot2lib Qt5::Test)

add_test(NAME SmoothTest COMMAND SmoothTest)
