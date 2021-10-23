project(labplot2)

# minimum may be 3.16.0 for FindIcoTool.cmake and FindSharedMimeInfo.cmake
cmake_minimum_required (VERSION 3.2.0 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(QT_MINIMUM_VERSION 5.6.0)
set(KF5_MIN_VERSION "5.16.0")

set(APPLE_SUPPRESS_X11_WARNING ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
# build type: "release", "debug", "debugfull"
string (TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS
	Concurrent
	Gui
# 	Qml
# 	Quick
# 	QuickWidgets
	PrintSupport
	Sql
	Svg
	Widgets
	Test
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
	KIO
	TextWidgets
	WidgetsAddons
	XmlGui
	NewStuffCore
	NewStuff
OPTIONAL_COMPONENTS
	Service
	Parts
	SyntaxHighlighting
)
# TODO: make NewStuff optional?
IF (KF5NewStuff_FOUND)
	add_definitions (-DHAVE_KF5_NEW_STUFF)
ELSE ()
        MESSAGE (STATUS "KF5 new stuff not found")
ENDIF ()
# TODO: service not used?
IF (NOT KF5Service_FOUND)
	MESSAGE (STATUS "KF5 service not found")
ENDIF ()
# TODO: parts not used?
IF (NOT KF5Parts_FOUND)
	MESSAGE (STATUS "KF5 parts not found")
ENDIF ()
IF (KF5SyntaxHighlighting_FOUND)
	add_definitions (-DHAVE_KF5_SYNTAX_HIGHLIGHTING)
ELSE ()
	MESSAGE (STATUS "KF5 syntax highlighting not found")
ENDIF ()

find_package(KUserFeedback)
IF (KUserFeedback_FOUND)
	MESSAGE (STATUS "Found KUserFeedback")
	add_definitions (-DHAVE_KUSERFEEDBACK)
ELSE ()
	MESSAGE (STATUS "KUserFeedback not found")
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

##########################################################

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION=\"2.9.0\")
add_definitions (-DCXX_COMPILER=\"${CMAKE_CXX_COMPILER_ID}\ ${CMAKE_CXX_COMPILER_VERSION}\")
add_definitions (-DCXX_COMPILER_FLAGS=\"${CMAKE_CXX_FLAGS}\")
add_definitions (-DQT_NO_URL_CAST_FROM_STRING)
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
      set(CLANG_TIDY_CHECKS "modernize-*,-modernize-use-trailing-return-type,clang-analyzer-*,-clang-analyzer-cplusplus*")
      #set(CLANG_TIDY_CHECKS "-*,modernize-*,clang-analyzer-*")
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

# get git commit hash
execute_process(
  COMMAND git describe --always --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions (-DGIT_COMMIT=\"${GIT_COMMIT}\")


### Options ######################################
option(ENABLE_CANTOR "Build with Cantor support" ON)
option(ENABLE_FFTW "Build with FFTW support" ON)
option(ENABLE_HDF5 "Build with HDF5 support" ON)
option(ENABLE_NETCDF "Build with NetCDF support" ON)
option(ENABLE_FITS "Build with FITS support" ON)
option(ENABLE_LIBCERF "Build with libcerf support" ON)
option(ENABLE_LIBORIGIN "Build with liborigin support" ON)
option(ENABLE_ROOT "Build with ROOT (CERN) support" ON)
option(ENABLE_READSTAT "Build with ReadStat support" ON)
option(ENABLE_MATIO "Build with Matio support" ON)
option(ENABLE_TESTS "Build with tests" ON)
option(ENABLE_MQTT "Build with MQTT support" ON)
option(ENABLE_QTSERIALPORT "Build with QtSerialPort support" ON)
option(ENABLE_DISCOUNT "Build with Discount support" ON)
option(ENABLE_REPRODUCIBLE "Enable reproducible builds" OFF)

### OS macros ####################################
IF (WIN32)
	add_definitions (-DHAVE_WINDOWS)
	find_library (PSAPI Psapi)
	message (STATUS "PSAPI: ${PSAPI}")
ENDIF ()

### GSL (required) ###############################
FIND_PACKAGE(GSL REQUIRED)

FIND_PACKAGE(Poppler "0.62.0" COMPONENTS Qt5)
IF (Poppler_FOUND)
	include_directories(${Poppler_INCLUDE_DIRS})
	# MESSAGE (STATUS "Poppler libraries: ${Poppler_LIBRARIES}")
	add_definitions (-DHAVE_POPPLER)
ENDIF ()

### liborigin (included) ###############################
IF (ENABLE_LIBORIGIN)
	FIND_PACKAGE (LibOrigin)
	IF (NOT LIBORIGIN_FOUND)	# use own version
		IF (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "debugfull")
			MESSAGE (STATUS "Origin project import (through internal liborigin) enabled (parser logging enabled)")
			SET (ENABLE_ORIGIN_PARSER_LOG TRUE)
		ELSE ()
			MESSAGE (STATUS "Origin project import (through internal liborigin) enabled (parser logging disabled)")
		ENDIF ()
	ENDIF ()

	add_definitions (-DHAVE_LIBORIGIN)
ELSE ()
	MESSAGE (STATUS "Origin project import DISABLED")
ENDIF ()

### Cantorlibs (optional) ###############################
IF (ENABLE_CANTOR)
	FIND_PACKAGE (Cantor)

	IF (Cantor_FOUND)
		MESSAGE (STATUS "Found Cantor Library ${Cantor_VERSION}")

		IF (${Cantor_VERSION} VERSION_GREATER "19.11")
			add_definitions (-DHAVE_CANTOR_LIBS)
			set(RECENT_CANTOR TRUE)
		ELSE ()
			MESSAGE (STATUS "Cantor Library ${Cantor_VERSION} TOO OLD. Minimum usable version is 19.12")
		ENDIF ()
		IF (${Cantor_VERSION} VERSION_GREATER "20.08.9")
			add_definitions (-DHAVE_NEW_CANTOR_LIBS)
		ENDIF ()
	ELSE ()
		MESSAGE (STATUS "Cantor Library NOT FOUND")
	ENDIF ()
	ELSE ()
		MESSAGE (STATUS "Cantor Library DISABLED")
ENDIF ()

### FFTW (optional) #####################################
IF (ENABLE_FFTW)
	FIND_PACKAGE (FFTW3)
	IF (FFTW3_FOUND)
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
SET_PACKAGE_PROPERTIES (HDF5 PROPERTIES
	DESCRIPTION "Reading and writing self describing array data"
	URL "https://www.hdfgroup.org/solutions/hdf5/"
)
IF (HDF5_FOUND)
	add_definitions (-DHAVE_HDF5)
	IF (MSVC_FOUND)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_BUILT_AS_DYNAMIC_LIB")
	ENDIF ()
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
	SET(HDF5_LIBRARIES "")
ENDIF ()
ELSE ()
	MESSAGE (STATUS "Hierarchical Data Format (HDF5) Library DISABLED")
ENDIF ()

### NETCDF (optional) #############################
IF (ENABLE_NETCDF)
	FIND_PACKAGE(netCDF)
	SET_PACKAGE_PROPERTIES(netCDF PROPERTIES
	    DESCRIPTION "Interfaces for array-oriented data access"
	    URL "https://www.unidata.ucar.edu/software/netcdf/"
	)
	IF (netCDF_FOUND)
		add_definitions (-DHAVE_NETCDF)
		# netCDF on Windows may link to zip.dll
		find_library(Zip_LIBRARIES NAMES zip)
		if (Zip_LIBRARIES)
			MESSAGE (STATUS "Found Zip Library ${Zip_LIBRARIES}")
		endif ()
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
	find_package(Qt5Mqtt ${QT_MIN_VERSION} NO_MODULE)
	SET_PACKAGE_PROPERTIES (Qt5Mqtt PROPERTIES
		DESCRIPTION "Qt Module to implement MQTT protocol version 3.1 and 3.1.1"
		URL "https://github.com/qt/qtmqtt")
	IF (Qt5Mqtt_FOUND)
		MESSAGE (STATUS "Found MQTT Library")
		add_definitions (-DHAVE_MQTT)
	ELSE ()
		MESSAGE (STATUS "MQTT Library NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "MQTT Library DISABLED")
ENDIF ()

### QtSerialPort (optional) ###############################
IF (ENABLE_QTSERIALPORT)
	find_package(Qt5SerialPort ${QT_MIN_VERSION} NO_MODULE)
	SET_PACKAGE_PROPERTIES (Qt5SerialPort PROPERTIES
		DESCRIPTION "Qt Serial Port library"
		URL "https://doc.qt.io/qt-5/qtserialport-index.html"
		PURPOSE "Support basic functionality of serial ports.")
	IF (Qt5SerialPort_FOUND)
		MESSAGE (STATUS "Found Qt5SerialPort Library")
		add_definitions (-DHAVE_QTSERIALPORT)
	ELSE ()
		MESSAGE (STATUS "Qt5SerialPort Library NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Qt5SerialPort Library DISABLED")
ENDIF ()

### FITS (optional) ###############################
IF (ENABLE_FITS)
	FIND_PACKAGE (CFitsio)
	SET_PACKAGE_PROPERTIES (CFitsio PROPERTIES
		DESCRIPTION "FITS IO Library"
		URL "https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html"
		PURPOSE "Support for the FITS (Flexible Image Transport System) data format.")
	IF (CFITSIO_FOUND)
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
		add_definitions (-DHAVE_LIBCERF)
	ELSE ()
		MESSAGE (STATUS "libcerf library NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "libcerf library DISABLED")
ENDIF ()

### ROOT (optional) #############################
IF (ENABLE_ROOT)
	FIND_PACKAGE(ZLIB)
	SET_PACKAGE_PROPERTIES (ZLIB PROPERTIES
		DESCRIPTION "General purpose compression library"
		URL "https://www.zlib.net/"
	)
	FIND_PACKAGE(LZ4)
	IF (ZLIB_FOUND AND LZ4_FOUND)
		MESSAGE (STATUS "Found ZIP libraries ZLIB and LZ4 (needed for ROOT importer)")
		add_definitions (-DHAVE_ZIP)
	ELSE ()
		MESSAGE (STATUS "ZIP libraries ZLIB or LZ4 (needed for ROOT importer) NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "ROOT (CERN) importer DISABLED")
ENDIF ()

### ReadStat (optional) #############################
IF (ENABLE_READSTAT)
	FIND_PACKAGE (ReadStat)
	IF (NOT READSTAT_FOUND AND NOT WIN32)	# own version not on Windows
		MESSAGE (STATUS "ReadStat library NOT FOUND. Building own version")
		set(BUILD_READSTAT TRUE)

		# link own readstat with iconv
		FIND_LIBRARY (ICONV_LIBRARIES NAMES iconv libiconv libiconv-2)
		IF (ICONV_LIBRARIES) # non-glibc
			MESSAGE (STATUS "Iconv library FOUND (${ICONV_LIBRARIES})")
			set(READSTAT_LIBRARIES ${CMAKE_BINARY_DIR}/src/3rdparty/install/lib/libreadstat.a ${ICONV_LIBRARIES})
		ELSE ()
			MESSAGE (STATUS "Iconv library NOT FOUND")
			set(READSTAT_LIBRARIES ${CMAKE_BINARY_DIR}/src/3rdparty/install/lib/libreadstat.a)
		ENDIF ()
		include_directories (${CMAKE_BINARY_DIR}/src/3rdparty/install/include)
	ENDIF ()
	IF (READSTAT_FOUND OR NOT WIN32)
		add_definitions (-DHAVE_READSTAT)
	ELSE ()
		SET(READSTAT_LIBRARIES "")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "ReadStat support DISABLED")
ENDIF ()

###  Matio (optional) ##############################################
IF (ENABLE_MATIO)
	FIND_PACKAGE (Matio)
	IF (MATIO_FOUND)
		add_definitions (-DHAVE_MATIO)
	ELSE ()
		MESSAGE (STATUS "Matio library NOT FOUND.")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Matio support DISABLED")
ENDIF ()

### Discount (optional) #############################
IF (ENABLE_DISCOUNT)
	FIND_PACKAGE(Discount)
	SET_PACKAGE_PROPERTIES (Discount PROPERTIES
		DESCRIPTION "A C implementation of the Markdown markup language"
		URL "https://www.pell.portland.or.us/~orc/Code/discount/"
		TYPE OPTIONAL)
	IF (Discount_FOUND)
		add_definitions (-DHAVE_DISCOUNT)
	ELSE ()
		MESSAGE (STATUS "Discount library NOT FOUND.")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Discount DISABLED")
ENDIF ()

#################################################
IF (ENABLE_REPRODUCIBLE)
	add_definitions (-DREPRODUCIBLE_BUILD)
	message(STATUS "Reproducable build ENABLED")
ELSE ()
	message(STATUS "Reproducable build DISABLED")
ENDIF ()
#################################################
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
#add_subdirectory(lib)

if (ENABLE_TESTS)
    enable_testing(true)
    add_subdirectory(tests)
endif()

install(FILES org.kde.labplot2.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL)

find_package(KF5I18n CONFIG REQUIRED)
ki18n_install(po)
find_package(KF5DocTools CONFIG REQUIRED)
kdoctools_install(po)
