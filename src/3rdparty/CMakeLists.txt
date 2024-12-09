if(APPLE AND ENABLE_TOUCHBAR)
    add_subdirectory(kdmactouchbar)
endif()

if(ENABLE_LIBORIGIN AND NOT LIBORIGIN_FOUND)
    add_subdirectory(liborigin)
endif()

if(BUILD_READSTAT)
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
#if(WIN32)    # not used (own version on Windows)
#        set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/zlib-develop-mingw-fixed.zip)
#        set(ext_configure cmake ${ext_src} ${generator} -DBUILD_SHARED_LIBS=0 ${rc_compiler} ${install_prefix} ${build_type})
#        set(ext_install ${install_target})
#        set(ext_insource OFF )
#                set(zlib_library libzlibstatic.a)
#        set(ext_make ${kst_make_program})
#else()

    # --prefix=${install_dir} does not work with spaces
    set(ext_configure ${ext_src}/configure CFLAGS="-fPIC" --enable-static --disable-shared)
    set(ext_insource ON)
    #set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)

    set(patch_command git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/readstat-1.1.9.patch)
    set(readstat_library libreadstat.a)
#endif()
    ExternalProject_Add(3rdparty_readstat
        PREFIX ${build_dir}/readstat
        URL ${ext_url}
        SOURCE_DIR ${ext_src}
        CONFIGURE_COMMAND ${ext_configure}
        BUILD_IN_SOURCE ${ext_insource}
        PATCH_COMMAND ${patch_command}
        #BUILD_COMMAND ${ext_make}
        # libtool install does not work with spaces
        #INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
        INSTALL_COMMAND cp -av .libs/${readstat_library} ${install_dir}/lib/ && cp -av src/readstat.h ${install_dir}/include/
        BUILD_BYPRODUCTS ${install_dir}/lib/${readstat_library}
    )
endif()

if(BUILD_QXLSX)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(QXlsx EXCLUDE_FROM_ALL)
endif()

# preview.sty
install(FILES preview.sty DESTINATION ${KDE_INSTALL_DATADIR}/${PROJECT_NAME}/latex)

Include(FetchContent)

### Vector BLF ###############################
FIND_PACKAGE(PkgConfig REQUIRED)
# git download is not allowed on all system (set to ON if needed)
option(LOCAL_DBC_PARSER "Use local build of dbc parser" OFF)
option(LOCAL_VECTOR_BLF "Use local build of Vector BLF" OFF)
if(ENABLE_VECTOR_BLF)
  set(OPTION_RUN_DOXYGEN OFF CACHE BOOL "" FORCE) # disable running doxygen
  if(${LOCAL_DBC_PARSER})
      FetchContent_Declare(
        dbc_parser_cpp
        GIT_REPOSITORY https://github.com/LinuxDevon/dbc_parser_cpp.git
        GIT_TAG cc4e80c7b6e1f7a13cc47b45f226756f823888e2 # v0.4.0
      )
      # fix missing variable and keyword for fast_float
      if(NOT MSVC_FOUND)
          add_definitions (-DFASTFLOAT_VISUAL_STUDIO=0)
          add_definitions (-D__forceinline=inline)
      endif()
      # disable -Werror=maybe-uninitialized since gcc gives false positive warnings when address sanitizer is used
      if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=maybe-uninitialized")
      endif()
      # dbc is the root project name
      # https://gitlab.kitware.com/cmake/cmake/-/issues/19854
      set(CMAKE_PROJECT_dbc_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dbc_database_parser_disableTests.cmake")
      # don't install dbc_parser_cpp
      # see https://stackoverflow.com/questions/65527126/disable-install-for-fetchcontent
      # FetchContent_MakeAvailable(dbc_parser_cpp)
      FetchContent_GetProperties(dbc_parser_cpp)
      if(NOT dbc_parser_cpp_POPULATED)
          FetchContent_Populate(dbc_parser_cpp)
          add_subdirectory(${dbc_parser_cpp_SOURCE_DIR} ${dbc_parser_cpp_BINARY_DIR} EXCLUDE_FROM_ALL)
      endif()
      # headerfiles of the projects are referring to <libdbc/someheader.h> and therefore
      # the path must be added
      target_include_directories(labplotbackendlib PUBLIC ${dbc_parser_cpp_SOURCE_DIR}/include)
      target_link_libraries(labplotbackendlib dbc)
      target_compile_definitions(labplotbackendlib PRIVATE HAVE_DBC_PARSER)
      target_compile_definitions(labplotlib PUBLIC HAVE_DBC_PARSER)
  else ()
    pkg_check_modules(dbc IMPORTED_TARGET GLOBAL dbc)
    if(dbc_FOUND)
        target_link_libraries(labplotbackendlib PkgConfig::dbc)
        target_compile_definitions(labplotbackendlib PRIVATE HAVE_DBC_PARSER)
        target_compile_definitions(labplotlib PUBLIC HAVE_DBC_PARSER)
    endif()
  endif()
  
  if(${LOCAL_VECTOR_BLF})
    FetchContent_Declare(
        vector_blf
        #GIT_REPOSITORY https://github.com/Technica-Engineering/vector_blf.git
        #GIT_TAG 02f0a2fe1f7915da1e24025869b98f6d15de937b
        GIT_REPOSITORY https://github.com/Murmele/vector_blf.git
        GIT_TAG 2c1a8e649328f2e06ce4d45c6223628bc39fd178
      )
      set(CMAKE_PROJECT_Vector_BLF_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vector_blf_config.cmake")
      # TODO: link static and don't install (see dbc_parser_cpp)
      FetchContent_MakeAvailable(vector_blf)
      # headerfiles of the projects are referring to <Vector/BLF/someheader.h> and therefore
      # the path must be added
      set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
          ${vector_blf_SOURCE_DIR}/src
          ${vector_blf_BINARY_DIR}/src) # config.h is in there
      target_include_directories(Vector_BLF PUBLIC ${vector_blf_SOURCE_DIR}/src
                          ${vector_blf_BINARY_DIR}/src) # config.h is in there
      target_link_libraries(labplotbackendlib Vector_BLF)
      target_compile_definitions(labplotbackendlib PRIVATE HAVE_VECTOR_BLF)
      target_compile_definitions(labplotlib PUBLIC HAVE_VECTOR_BLF)
      add_definitions (-DHAVE_VECTOR_BLF)
  else()
    pkg_check_modules(Vector_BLF IMPORTED_TARGET GLOBAL Vector_BLF)
    if(Vector_BLF_FOUND)
        target_link_libraries(labplotbackendlib PkgConfig::Vector_BLF)
        target_compile_definitions(labplotbackendlib PRIVATE HAVE_VECTOR_BLF)
        target_compile_definitions(labplotlib PUBLIC HAVE_VECTOR_BLF)
        add_definitions (-DHAVE_VECTOR_BLF)
    endif()
  endif()
else()
    add_definitions (-DVECTOR_BLF_DISABLED)
endif()

if(LOCAL_QT_ADVANCED_DOCK_DOWNLOAD)
    FetchContent_Declare(
        qt_advanced_dock
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG a940fc29c6d1fbcb5ee1e2fecf86f20bb86a7fb6
    )
    set(CMAKE_PROJECT_QtADS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtADS_config.cmake")
    # don't install qt_advanced_dock
    # see https://stackoverflow.com/questions/65527126/disable-install-for-fetchcontent
    # FetchContent_MakeAvailable(qt_advanced_dock)
    FetchContent_GetProperties(qt_advanced_dock)
    if(NOT qt_advanced_dock_POPULATED)
        FetchContent_Populate(qt_advanced_dock)
        add_subdirectory(${qt_advanced_dock_SOURCE_DIR} ${qt_advanced_dock_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
elseif(LOCAL_QT_ADVANCED_DOCK_COPY)
    # https://cmake.org/cmake/help/latest/variable/CMAKE_PROJECT_INCLUDE.html
    set(CMAKE_PROJECT_QtADS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtADS_config.cmake")
    set(ADS_VERSION ${labplot_VERSION})
    add_subdirectory(Qt-Advanced-Docking-System EXCLUDE_FROM_ALL)
endif()

if(LOCAL_MCAP_DOWNLOAD)
    FetchContent_Declare(mcap
        URL https://github.com/foxglove/mcap/archive/refs/tags/releases/cpp/v1.4.0.tar.gz
        URL_HASH SHA256=64ff3e51119f37ffcfaf9deecbd987a7cb4d4d9035d74a3fd3773395a470fda1 # v1.4.0
    )
    FetchContent_MakeAvailable(mcap)
    #set(_mcap_include_dir ${mcap_SOURCE_DIR}/cpp/mcap/include)
    target_include_directories(labplotbackendlib PUBLIC ${mcap_SOURCE_DIR}/cpp/mcap/include)
else ()
    target_include_directories(labplotbackendlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mcap/include)
endif()
