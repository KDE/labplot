add_executable(ProjectImportTest ProjectImportTest.cpp)

if(ENABLE_LIBORIGIN)
    if(LIBORIGIN_FOUND)
    # if system headers are missing: use the provided
        target_include_directories(ProjectImportTest PRIVATE ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
        else() # own version
        target_include_directories(ProjectImportTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin")
    endif()
endif()

target_link_libraries(ProjectImportTest labplotmetatest)

add_test(NAME ProjectImportTest COMMAND ProjectImportTest)
