IF (APPLE)
	add_subdirectory(kdmactouchbar)
ENDIF ()

IF (ENABLE_LIBORIGIN AND NOT LIBORIGIN_FOUND)
	add_subdirectory(liborigin)
ENDIF ()

IF (BUILD_READSTAT)
	include(ExternalProject)

	set(build_dir ${CMAKE_BINARY_DIR}/src/3rdparty)
	set(install_dir ${build_dir}/install)
	file(MAKE_DIRECTORY ${install_dir}/lib)
	file(MAKE_DIRECTORY ${install_dir}/include)

	set(ext_src ${build_dir}/readstat/src/readstat)
	set(ext_version "1.1.6")
#if(WIN32)	# not used (own version on Windows)
#        set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/zlib-develop-mingw-fixed.zip)
#        set(ext_configure cmake ${ext_src} ${generator} -DBUILD_SHARED_LIBS=0 ${rc_compiler} ${install_prefix} ${build_type})
#        set(ext_install ${install_target})
#        set(ext_insource OFF )
#                set(zlib_library libzlibstatic.a)
#        set(ext_make ${kst_make_program})
#else()
        set(ext_url https://github.com/WizardMac/ReadStat/releases/download/v${ext_version}/readstat-${ext_version}.tar.gz)
	# --prefix=${install_dir} does not work with spaces
        set(ext_configure ${ext_src}/configure --enable-static --disable-shared)
	set(ext_insource ON)
	#set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)

	set(readstat_library libreadstat.a)
#endif()
	ExternalProject_Add(3rdparty_readstat
		PREFIX ${build_dir}/readstat
		URL ${ext_url}
		SOURCE_DIR ${ext_src}
		CONFIGURE_COMMAND ${ext_configure}
		BUILD_IN_SOURCE ${ext_insource}
		#BUILD_COMMAND ${ext_make}
		# libtool install does not work with spaces
		#INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
		INSTALL_COMMAND cp -av .libs/${readstat_library} ${install_dir}/lib/ && cp -av src/readstat.h ${install_dir}/include/)
ENDIF ()

IF (BUILD_EXCEL)
        include(ExternalProject)

        set(build_dir ${CMAKE_BINARY_DIR}/src/3rdparty )

        set(install_dir ${build_dir}/install)
        file(MAKE_DIRECTORY ${install_dir}/lib)
        file(MAKE_DIRECTORY ${install_dir}/include)

        set(ext_src ${build_dir}/QXlsx/src/QXlsx )
        set(ext_insource ON)
        set(install_dir ${build_dir}/install)
        set(qxlsx_library QXlsx.lib )
        if(WIN32)
        ExternalProject_Add(3rdparty_QXlsx
                GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
                PREFIX ${build_dir}/QXlsx
                SOURCE_DIR ${ext_src}/QXlsx
                SOURCE_SUBDIR QXlsx
                BUILD_IN_SOURCE ${ext_insource}
                #INSTALL_COMMAND "")
                INSTALL_COMMAND Robocopy . ${install_dir}/lib ${qxlsx_library} /njs /njh & Robocopy header ${install_dir}/include /njs /njh /MIR )
        else()
        ExternalProject_Add(3rdparty_QXlsx
                GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
                PREFIX ${build_dir}/QXlsx
                SOURCE_DIR ${ext_src}/QXlsx
                SOURCE_SUBDIR QXlsx
                BUILD_IN_SOURCE ${ext_insource}
                INSTALL_COMMAND "")
                #INSTALL_COMMAND  cp -av ./${qxlsx_library} ${install_dir}/lib && cp  ./header/* ${install_dir}/include )
        endif()

        include_directories(${install_dir}/include )
ENDIF ()

# preview.sty
install(FILES preview.sty DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME}/latex)

