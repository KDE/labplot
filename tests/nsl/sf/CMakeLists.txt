INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR})

# basic functions
add_executable (nslsfbasictest NSLSFBasicTest.cpp)

target_link_libraries(nslsfbasictest Qt5::Test)
target_link_libraries(nslsfbasictest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslsfbasictest labplot2lib)

add_test(NAME nslsfbasictest COMMAND nslsfbasictest)

# window functions
add_executable (nslsfwindowtest NSLSFWindowTest.cpp)

target_link_libraries(nslsfwindowtest Qt5::Test)
target_link_libraries(nslsfwindowtest ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})

target_link_libraries(nslsfwindowtest labplot2lib)

add_test(NAME nslsfwindowtest COMMAND nslsfwindowtest)
