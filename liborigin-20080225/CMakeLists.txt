add_library (origin OPJFile.cpp)
set_target_properties(origin PROPERTIES VERSION 0.0.4 SOVERSION 0 )

#install(TARGETS origin DESTINATION lib)

add_executable(opj2dat opj2dat.cpp)
target_link_libraries (opj2dat origin)

install(TARGETS opj2dat DESTINATION bin)

