set(QT_USE_QTTEST TRUE)
include(${QT_USE_FILE})

set(TEST_GLOBAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

function(CREATE_TEST TEST_NAME)
	include_directories(
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/../../
	)

	file(GLOB TEST_SRCS "*.cpp")
	file(GLOB TEST_HDRS "*.h")

	add_executable(${TEST_NAME} ${TEST_SRCS} ${TEST_MOC})
	target_link_libraries(${TEST_NAME}
		liblabplot2_shared
		${KDE4_KDEUI_LIBS}
		${KDE4_KIO_LIBS}
		${GSL_LIBRARIES}
		${GSL_CBLAS_LIBRARIES}
		${HDF5_LIBRARY}
		${NETCDF_LIBRARY}
		${VTK_LIBRARIES}
		${OPENGL_LIBRARIES}
		${QT_LIBRARIES}
		${QT_QTTEST_LIBRARY}
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

add_subdirectory(Surface3DDock)
add_subdirectory(Surface3D)