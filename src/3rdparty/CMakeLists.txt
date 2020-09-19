IF (APPLE)
	add_subdirectory(kdmactouchbar)
ENDIF ()

IF (BUILD_READSTAT)
	include(ExternalProject)

	set(build_dir "${CMAKE_BINARY_DIR}/src/3rdparty")
	set(install_dir "${build_dir}/install")
	file(MAKE_DIRECTORY "${install_dir}")
	set(install_prefix -DCMAKE_INSTALL_PREFIX="${install_dir}")

	set(ext_src ${build_dir}/readstat/src/readstat)
#if(WIN32)
#        set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/zlib-develop-mingw-fixed.zip)
#        set(ext_configure cmake ${ext_src} ${generator} -DBUILD_SHARED_LIBS=0 ${rc_compiler} ${install_prefix} ${build_type})
#        set(ext_install ${install_target})
#        set(ext_insource OFF )
#                set(zlib_library libzlibstatic.a)
#        set(ext_make ${kst_make_program})
#else()
        set(ext_url https://github.com/WizardMac/ReadStat/releases/download/v1.1.4/readstat-1.1.4.tar.gz)
        set(ext_configure ${ext_src}/configure --enable-static --disable-shared --prefix=${install_dir})
        set(ext_install install)
	set(ext_insource ON)
	set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)
        set(readstat_library libreadstat.a)
#endif()
	ExternalProject_Add(3rdparty_readstat
		PREFIX ${build_dir}/readstat
		URL ${ext_url}
		SOURCE_DIR ${ext_src}
		CONFIGURE_COMMAND ${ext_configure}
		BUILD_IN_SOURCE ${ext_insource}
		BUILD_COMMAND ${ext_make}
		INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} ${ext_install})

	#kst_external_static_lib(zlib_build zlib ${zlib_library})
ENDIF ()

# preview.sty
install(FILES preview.sty DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/latex)

