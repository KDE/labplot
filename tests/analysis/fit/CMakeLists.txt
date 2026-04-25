add_executable(FitTest FitTest.cpp)

target_link_libraries(FitTest labplotlib labplotbackendlib labplottest ${LIBCERF_LIBRARIES})

add_test(NAME FitTest COMMAND FitTest)
