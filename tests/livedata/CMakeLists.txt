add_executable(LiveDataTest LiveDataTest.cpp)

target_link_libraries(LiveDataTest labplotbackendlib labplotlib labplottest)

add_test(NAME LiveDataTest COMMAND LiveDataTest)
