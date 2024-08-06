add_executable(FitTest FitTest.cpp)

target_link_libraries(FitTest labplotbackendlib labplotlib labplottest ${LIBCERF_LIBRARIES})

add_test(NAME FitTest COMMAND FitTest)
