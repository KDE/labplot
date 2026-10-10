add_executable(FitTest FitTest.cpp)

target_link_libraries(FitTest labplotbackendlib labplotlib labplotmetatest ${LIBCERF_LIBRARIES})

add_test(NAME FitTest COMMAND FitTest)

add_executable(PiecewiseLinearFitTest PiecewiseLinearFitTest.cpp)

target_link_libraries(PiecewiseLinearFitTest labplotbackendlib labplotlib labplotmetatest ${LIBCERF_LIBRARIES})

add_test(NAME PiecewiseLinearFitTest COMMAND PiecewiseLinearFitTest)
