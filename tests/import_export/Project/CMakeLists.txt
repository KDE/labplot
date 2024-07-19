add_executable (ProjectImportTest ProjectImportTest.cpp)

IF (ENABLE_LIBORIGIN)
	IF (LIBORIGIN_FOUND)
	# if system headers are missing: use the provided
		target_include_directories( ProjectImportTest PRIVATE ${LIBORIGIN_INCLUDE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin" )
        ELSE () # own version
		target_include_directories( ProjectImportTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/liborigin" )
	ENDIF()
ENDIF ()

target_link_libraries(ProjectImportTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ProjectImportTest COMMAND ProjectImportTest)
