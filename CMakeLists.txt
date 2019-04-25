project(labplot2)

# minimum 3.2.0 for FindGSL.cmake
cmake_minimum_required (VERSION 3.2.0 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(QT_MINIMUM_VERSION 5.6.0)
set(KF5_MIN_VERSION "5.16.0")

set(APPLE_SUPPRESS_X11_WARNING ON)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
# build type: "release", "debug", "debugfull"
string (TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS
	Concurrent
	Gui
	PrintSupport
	Sql
	Svg
	Widgets
	Test
        SerialPort
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
	Archive
	Completion
	Config
	ConfigWidgets
	CoreAddons
	Crash
	DocTools
	I18n
	IconThemes
	TextWidgets
	WidgetsAddons
	XmlGui
OPTIONAL_COMPONENTS
	NewStuff
	Service
	Parts
	SyntaxHighlighting
)
IF (Qt5SerialPort_FOUND)
        MESSAGE (STATUS "Found Qt5 SerialPort")
ELSE ()
        MESSAGE (STATUS "Qt5 SerialPort not found")
ENDIF ()
IF (KF5NewStuff_FOUND)
        MESSAGE (STATUS "Found KF5 new stuff")
	add_definitions (-DHAVE_KF5_NEW_STUFF)
ELSE ()
        MESSAGE (STATUS "KF5 new stuff not found")
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
        MESSAGE (STATUS "Found KF5 syntax highlighting")
	add_definitions (-DHAVE_KF5_SYNTAX_HIGHLIGHTING)
ELSE ()
        MESSAGE (STATUS "KF5 syntax highlighting not found")
ENDIF ()

find_package(BISON REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

### compiler flags ######################################
set (GENERIC_FLAGS "-Wall -Wextra -Wundef -Wpointer-arith -Wunreachable-code -Wunused -Wdeprecated-declarations -fno-omit-frame-pointer -fstack-protector")
set (GENERIC_GNU_FLAGS "-O2 -Wcast-align -Wswitch-enum -fvisibility=default")
set (GENERIC_C_FLAGS "${GENERIC_FLAGS} -fno-exceptions")
# liborigin needs exceptions
set (GENERIC_CXX_FLAGS "${GENERIC_FLAGS} -fexceptions")

if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
	message(STATUS "GNU C compiler detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS} ${GENERIC_GNU_FLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
	message(STATUS "Clang C compiler detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE ${GENERIC_C_FLAGS} ${GENERIC_GNU_FLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
	message(STATUS "Intel C compiler detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -O3 ${GENERIC_C_FLAGS}")
elseif ("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")
	message(STATUS "PGI C compiler detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -O3 -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -Minform=inform -Mbounds -Mchkstk")
# " x" postfix to work around a bug in CMake that causes "MSVC" to translate to something completely different
elseif (("${CMAKE_C_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
	message(STATUS "MSVC C compiler detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -W3")
	set(MSVC_FOUND TRUE)
else ()
	message(STATUS "UNKNOWN C compiler, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	message(STATUS "GNU C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	message(STATUS "Clang C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
	message(STATUS "Intel C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -std=c++11 ${GENERIC_CXX_FLAGS}")
	#-std=c++0x comes with cmake's general flags, deprecated in icc, remove it
	string(REPLACE "-std=c++0x" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
	message(STATUS "PGI C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -O3 -std=c++11 -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -Minform=inform -Mbounds -Mchkstk")
# " x" postfix to work around a bug in CMake that causes "MSVC" to translate to something completely different
elseif (("${CMAKE_CXX_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
	message(STATUS "MSVC C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -W3 -DPSAPI_VERSION=1")
	set(MSVC_FOUND TRUE)
else ()
	message(STATUS "UNKNOWN C++ compiler, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS}")
endif ()

##########################################################ESC[m


add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION=\"2.6.0\")
# add_definitions (-DLDEBUG='1')
set(BUILD_SHARED_LIBS true)

#cmake_policy(SET CMP0002 OLD)
IF (CMAKE_VERSION VERSION_EQUAL "3.3" OR CMAKE_VERSION VERSION_GREATER "3.3")
	cmake_policy(SET CMP0063 NEW)
ENDIF()

if (CMAKE_VERSION VERSION_GREATER "3.5")
  set(ENABLE_CLANG_TIDY OFF CACHE BOOL "Add clang-tidy automatically to builds")
  if (ENABLE_CLANG_TIDY)
    find_program (CLANG_TIDY_EXE NAMES "clang-tidy" PATHS /usr/bin)
    if (CLANG_TIDY_EXE)
      message(STATUS "Clang-tidy supported, found and enabled: ${CLANG_TIDY_EXE}")
      set(CLANG_TIDY_CHECKS "-*,modernize-*")
      # -extra-arg=--std=c++11
      set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*'"
        CACHE STRING "" FORCE)
    else()
      message(AUTHOR_WARNING "clang-tidy not found!")
      set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif()
   else()
    message(STATUS "Clang-tidy supported but disabled")
  endif()
endif()

### Options ######################################
option(ENABLE_CANTOR "Build with Cantor support" ON)
option(ENABLE_FFTW "Build with FFTW support" ON)
option(ENABLE_HDF5 "Build with HDF5 support" ON)
option(ENABLE_NETCDF "Build with NetCDF support" ON)
option(ENABLE_FITS "Build with FITS support" ON)
option(ENABLE_LIBCERF "Build with libcerf support" ON)
option(ENABLE_LIBORIGIN "Build with liborigin support" ON)
option(ENABLE_ROOT "Build with ROOT (CERN) support" ON)
option(ENABLE_TESTS "Build with tests" ON)
option(ENABLE_MQTT "Build with MQTT support" ON)

### OS macros ####################################
IF (WIN32)
	add_definitions (-DHAVE_WINDOWS)
	find_library (PSAPI Psapi)
	message (STATUS "PSAPI: ${PSAPI}")
ENDIF ()

### GSL (required) ###############################
FIND_PACKAGE(GSL REQUIRED)

### liborigin (included) ###############################
IF (ENABLE_LIBORIGIN)
	add_definitions (-DHAVE_LIBORIGIN)

	IF (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "debugfull")
		MESSAGE (STATUS "Origin project import (through internal liborigin) enabled (parser logging enabled)")
		SET (ENABLE_ORIGIN_PARSER_LOG TRUE)
	ELSE ()
		MESSAGE (STATUS "Origin project import (through internal liborigin) enabled (parser logging disabled)")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Origin project import DISABLED")
ENDIF ()

### Cantorlibs (optional) ###############################
IF (ENABLE_CANTOR)
FIND_LIBRARY (CANTOR_LIBS cantorlibs)
FIND_PATH (CANTOR_INCLUDE_DIR cantor/worksheetaccess.h
	/usr/include
	/usr/local/include
)
IF (CANTOR_LIBS AND CANTOR_INCLUDE_DIR AND KF5Service_FOUND AND KF5Parts_FOUND)
	SET (CANTOR_LIBS_FOUND TRUE)
	FIND_PATH (CANTOR_VERSION_FILE cantor/cantorlibs_version.h
		/usr/include
		/usr/local/include
	)
	IF (NOT CANTOR_VERSION_FILE)
		add_definitions (-DOLD_CANTORLIBS_VERSION)
	ENDIF()
ELSE ()
	SET (CANTOR_LIBS_FOUND FALSE)
	SET (CANTOR_LIBS "")
ENDIF()
IF (CANTOR_LIBS_FOUND)
	MESSAGE (STATUS "Found Cantor Library: ${CANTOR_INCLUDE_DIR} ${CANTOR_LIBS}")
	add_definitions (-DHAVE_CANTOR_LIBS)
ELSE ()
	MESSAGE (STATUS "Cantor Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "Cantor Library DISABLED")
ENDIF ()

### FFTW (optional) #####################################
IF (ENABLE_FFTW)
FIND_LIBRARY (FFTW_LIBRARIES fftw3
        PATHS
        /usr/lib
        /usr/local/lib
)
FIND_PATH (FFTW_INCLUDE_DIR fftw3.h
	/usr/include
	/usr/local/include
)
IF (FFTW_LIBRARIES AND FFTW_INCLUDE_DIR)
	SET (FFTW_FOUND TRUE)
ELSE ()
	SET (FFTW_FOUND FALSE)
ENDIF ()
IF (FFTW_FOUND)
	MESSAGE (STATUS "Found FFTW 3 Library: ${FFTW_INCLUDE_DIR} ${FFTW_LIBRARIES}")
	add_definitions (-DHAVE_FFTW3)
ELSE ()
	MESSAGE (STATUS "FFTW 3 Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "FFTW 3 Library DISABLED")
ENDIF ()

### HDF5 (optional) ##############################
IF (ENABLE_HDF5)
FIND_PACKAGE(HDF5 COMPONENTS C)
IF (HDF5_FOUND)
	add_definitions (-DHAVE_HDF5)
	IF (HDF5_VERSION VERSION_GREATER "1.9")
		add_definitions (-DHAVE_AT_LEAST_HDF5_1_10_0)
	ENDIF ()
	IF (HDF5_VERSION VERSION_GREATER "1.10.0.1")
		add_definitions (-DHAVE_AT_LEAST_HDF5_1_10_0)
		add_definitions (-DHAVE_AT_LEAST_HDF5_1_10_1)
	ENDIF ()
	include_directories (${HDF5_INCLUDE_DIRS})
ELSE ()
	MESSAGE (STATUS "Hierarchical Data Format (HDF5) Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "Hierarchical Data Format (HDF5) Library DISABLED")
ENDIF ()

### NETCDF (optional) #############################
IF (ENABLE_NETCDF)
FIND_LIBRARY (NETCDF_LIBRARY netcdf
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_PATH (NETCDF_INCLUDE_DIR netcdf.h
	/usr/include
	/usr/local/include
)
IF (NETCDF_LIBRARY AND NETCDF_INCLUDE_DIR)
	SET (NETCDF_FOUND TRUE)
ELSE ()
	SET (NETCDF_FOUND FALSE)
ENDIF ()
IF (NETCDF_FOUND)
	MESSAGE (STATUS "Found Network Common Data Format (NetCDF) Library: ${NETCDF_INCLUDE_DIR} ${NETCDF_LIBRARY}")
	add_definitions (-DHAVE_NETCDF)
ELSE ()
	MESSAGE (STATUS "Network Common Data Format (NetCDF) Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "Network Common Data Format (NetCDF) Library DISABLED")
ENDIF ()

### MQTT (optional) ###############################
IF (ENABLE_MQTT)
# ATTENTION: unit test uses qWaitFor() which needs Qt >= 5.10
# avoid warning for the moment using QUIET
find_package(Qt5Mqtt ${QT_MIN_VERSION} QUIET NO_MODULE)
IF (Qt5Mqtt_FOUND)
	MESSAGE (STATUS "Found MQTT Library")
	add_definitions (-DHAVE_MQTT)
ELSE ()
	MESSAGE (STATUS "MQTT Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "MQTT Library DISABLED")
ENDIF ()

### FITS (optional) ###############################
IF (ENABLE_FITS)
FIND_LIBRARY (CFITSIO_LIBRARY cfitsio
	PATHS
	/usr/lib
	/usr/local/lib
	$ENV{CFITSIO}
)
FIND_PATH (CFITSIO_INCLUDE_DIR fitsio.h
	/usr/include
	/usr/include/cfitsio
	/usr/local/include
	$ENV{CFITSIO}
)
IF (CFITSIO_LIBRARY AND CFITSIO_INCLUDE_DIR)
        SET (CFITSIO_FOUND TRUE)
ELSE ()
        SET (CFITSIO_FOUND FALSE)
ENDIF ()
IF (CFITSIO_FOUND)
	MESSAGE (STATUS "Found Flexible Image Transport System Data Format (FITS) Library: ${CFITSIO_INCLUDE_DIR} ${CFITSIO_LIBRARY}")
	add_definitions (-DHAVE_FITS)
	include_directories (${CFITSIO_INCLUDE_DIR})
ELSE ()
	MESSAGE (STATUS "Flexible Image Transport System Data Format (FITS) Library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "Flexible Image Transport System Data Format (FITS) Library DISABLED")
ENDIF ()

### LIBCERF (optional) #############################
IF (ENABLE_LIBCERF)
FIND_PACKAGE (LIBCERF)
IF (LIBCERF_FOUND)
	MESSAGE (STATUS "Found libcerf Library: ${LIBCERF_INCLUDE_DIR} ${LIBCERF_LIBRARY}")
	add_definitions (-DHAVE_LIBCERF)
ELSE ()
	MESSAGE (STATUS "libcerf library NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "libcerf library DISABLED")
ENDIF ()

IF (ENABLE_ROOT)
FIND_PACKAGE(ZLIB)
FIND_LIBRARY(LZ4_LIBRARY lz4
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_PATH (LZ4_INCLUDE_DIR lz4.h
	/usr/include
	/usr/local/include
)
IF (LZ4_LIBRARY AND LZ4_INCLUDE_DIR)
	IF (EXISTS "${LZ4_INCLUDE_DIR}/lz4.h")
		file(STRINGS "${LZ4_INCLUDE_DIR}/lz4.h" LZ4_VERSION_MAJOR REGEX "^#define LZ4_VERSION_MAJOR.*")
		file(STRINGS "${LZ4_INCLUDE_DIR}/lz4.h" LZ4_VERSION_MINOR REGEX "^#define LZ4_VERSION_MINOR.*")
		file(STRINGS "${LZ4_INCLUDE_DIR}/lz4.h" LZ4_VERSION_RELEASE REGEX "^#define LZ4_VERSION_RELEASE.*")
		string(REGEX REPLACE ".*[ ].*([0-9]+).*$" "\\1" LZ4_VERSION_MAJOR "${LZ4_VERSION_MAJOR}")
		string(REGEX REPLACE ".*[ ].*([0-9]+).*$" "\\1" LZ4_VERSION_MINOR "${LZ4_VERSION_MINOR}")
		string(REGEX REPLACE ".*[ ].*([0-9]+).*$" "\\1" LZ4_VERSION_RELEASE "${LZ4_VERSION_RELEASE}")

		MESSAGE (STATUS "Found LZ4 library: ${LZ4_INCLUDE_DIR} ${LZ4_LIBRARY} (found version \"${LZ4_VERSION_MAJOR}.${LZ4_VERSION_MINOR}.${LZ4_VERSION_RELEASE}\")")
	ELSE ()
		MESSAGE (STATUS "Found LZ4 library: ${LZ4_INCLUDE_DIR} ${LZ4_LIBRARY} (version unknown)")
	ENDIF ()
	SET (LZ4_FOUND TRUE)
ELSE ()
	MESSAGE (STATUS "LZ4 library NOT FOUND")
	SET (LZ4_FOUND FALSE)
ENDIF ()
IF (ZLIB_FOUND AND LZ4_FOUND)
	MESSAGE (STATUS "Found ZIP libraries ZLIB and LZ4 (needed for ROOT importer)")
	add_definitions (-DHAVE_ZIP)
ELSE ()
	MESSAGE (STATUS "ZIP libraries ZLIB and LZ4 (needed for ROOT importer) NOT FOUND")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "ROOT (CERN) importer DISABLED")
ENDIF ()

#################################################
include(CheckFunctionExists)

CHECK_FUNCTION_EXISTS(random HAVE_RANDOM_FUNCTION)
#################################################
FIND_PATH (XLOCALE_INCLUDE_DIR xlocale.h
	/usr/include
	/usr/local/include
)
IF (XLOCALE_INCLUDE_DIR)
	add_definitions (-DHAVE_XLOCALE)
	include_directories (${XLOCALE_INCLUDE_DIR})
ENDIF()

add_subdirectory(data)
add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(doc)

IF (ENABLE_LIBORIGIN)
	add_subdirectory(liborigin)
ENDIF ()

if (ENABLE_TESTS)
    enable_testing(true)
    add_subdirectory(tests)
endif()

install(FILES org.kde.labplot2.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL)

