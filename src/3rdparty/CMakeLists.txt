IF (APPLE AND ENABLE_TOUCHBAR)
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
	# TODO: find easy (and portable) way to get latest version
	# curl -s https://api.github.com/repos/WizardMac/ReadStat/releases/latest | grep 'browser_download_url.*.tar.gz' | cut -d\" -f4
	# curl -s https://api.github.com/repos/WizardMac/ReadStat/releases/latest | jq --raw-output '.assets[0] | .browser_download_url'
	# execute_process(COMMAND "curl -s https://api.github.com/repos/WizardMac/ReadStat/releases/latest |   jq --raw-output '.assets[0] | .browser_download_url'" OUTPUT_VARIABLE ext_url)
	set(ext_version "1.1.9")
	set(ext_url https://github.com/WizardMac/ReadStat/releases/download/v${ext_version}/readstat-${ext_version}.tar.gz)
#if(WIN32)	# not used (own version on Windows)
#        set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/zlib-develop-mingw-fixed.zip)
#        set(ext_configure cmake ${ext_src} ${generator} -DBUILD_SHARED_LIBS=0 ${rc_compiler} ${install_prefix} ${build_type})
#        set(ext_install ${install_target})
#        set(ext_insource OFF )
#                set(zlib_library libzlibstatic.a)
#        set(ext_make ${kst_make_program})
#else()

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
		INSTALL_COMMAND cp -av .libs/${readstat_library} ${install_dir}/lib/ && cp -av src/readstat.h ${install_dir}/include/
		BUILD_BYPRODUCTS ${install_dir}/lib/${readstat_library}
	)
ENDIF ()

IF (BUILD_EXCEL)
	set(BUILD_SHARED_LIBS OFF)
	# git clone -b v1.4.3 https://github.com/QtExcel/QXlsx.git tmp
	# mv tmp/QXlsx .
	# mv tmp/{LICENSE, README.md} QXlsx
	# rm -rf tmp
	add_subdirectory(QXlsx)
ENDIF ()

# preview.sty
install(FILES preview.sty DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME}/latex)

Include(FetchContent)

### Vector BLF ###############################
FIND_PACKAGE(PkgConfig REQUIRED)
# git download is not allowed on all system (set to ON if needed)
option(LOCAL_DBC_PARSER "Use local build of dbc parser" OFF)
option(LOCAL_VECTOR_BLF "Use local build of Vector BLF" OFF)
if (ENABLE_VECTOR_BLF)
  set(OPTION_RUN_DOXYGEN OFF CACHE BOOL "" FORCE) # disable running doxygen
  if (${LOCAL_DBC_PARSER})
      FetchContent_Declare(
        dbc_parser_cpp
        GIT_REPOSITORY https://github.com/Murmele/dbc_parser_cpp.git
        GIT_TAG ca69d0dd08a03fc0210820a4f81bfd217ad25722
        #GIT_REPOSITORY https://github.com/LinuxDevon/dbc_parser_cpp.git
        #GIT_TAG        master
      )
      # dbc is the root project name
      # https://gitlab.kitware.com/cmake/cmake/-/issues/19854
      set(CMAKE_PROJECT_dbc_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dbc_database_parser_disableTests.cmake")
      FetchContent_MakeAvailable(dbc_parser_cpp)
      # headerfiles of the projects are referring to <libdbc/someheader.h> and therefore
      # the path must be added
      target_include_directories(labplot2lib PUBLIC ${dbc_parser_cpp_SOURCE_DIR}/include)
      target_link_libraries(labplot2lib dbc)
  else ()
    pkg_check_modules(dbc REQUIRED IMPORTED_TARGET GLOBAL dbc)
    target_link_libraries(labplot2lib PkgConfig::dbc)
  endif()
  target_compile_definitions(labplot2lib PRIVATE HAVE_DBC_PARSER)
  
  if (${LOCAL_VECTOR_BLF})
    FetchContent_Declare(
        vector_blf
        #GIT_REPOSITORY https://github.com/Technica-Engineering/vector_blf.git
        #GIT_TAG 02f0a2fe1f7915da1e24025869b98f6d15de937b
        GIT_REPOSITORY https://github.com/Murmele/vector_blf.git
        GIT_TAG b7a419c4d8edbf495ab309b57a380a2444ee2730
      )
      set(CMAKE_PROJECT_Vector_BLF_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_blf_config.cmake")
      FetchContent_MakeAvailable(vector_blf)
      # headerfiles of the projects are referring to <Vector/BLF/someheader.h> and therefore
      # the path must be added
      set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
          ${vector_blf_SOURCE_DIR}/src
          ${vector_blf_BINARY_DIR}/src) # config.h is in there
      target_include_directories(Vector_BLF PUBLIC ${vector_blf_SOURCE_DIR}/src
                          ${vector_blf_BINARY_DIR}/src) # config.h is in there
      target_link_libraries(labplot2lib Vector_BLF)
  else()
    pkg_check_modules(Vector_BLF REQUIRED IMPORTED_TARGET GLOBAL Vector_BLF)
    target_link_libraries(labplot2lib PkgConfig::Vector_BLF)
  endif()
    target_compile_definitions(labplot2lib PRIVATE HAVE_VECTOR_BLF)
endif()

