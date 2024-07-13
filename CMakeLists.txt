cmake_minimum_required (VERSION 3.17.0 FATAL_ERROR)
project(labplot2 VERSION 2.11.0 LANGUAGES CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
MESSAGE (STATUS "Found Qt version ${QT_VERSION_MAJOR}")
# see also -DQT_DISABLE_DEPRECATED_BEFORE in src/CMakeLists.txt
set(QT_MIN_VERSION "5.12.0")
set(KF_MAJOR_VERSION "5")
set(KF_MIN_VERSION "5.32.0")
#set(QT_VERSION_MAJOR "6")
if (QT_VERSION_MAJOR GREATER_EQUAL 6)
    set(KF_MIN_VERSION "5.240.0")
    find_package(ECM ${KF_MIN_VERSION} NO_MODULE)
    if (ECM_FOUND)
        set(KF_MAJOR_VERSION "6")
        set(QT_MIN_VERSION "6.2.2")		# openSUSE 15.4 version
    else ()
        MESSAGE (STATUS "KF6 not found! Trying Qt5")
	find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
	MESSAGE (STATUS "Found Qt version ${QT_VERSION_MAJOR}")
        set(KF_MIN_VERSION "5.32.0")
    endif ()
endif ()
set(QT_MAJOR_VERSION ${QT_VERSION_MAJOR})

set(APPLE_SUPPRESS_X11_WARNING ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddAppIcon)
#include(KDEClangFormat)
#include(GenerateExportHeader)
include(FeatureSummary)

# build type: "release", "debug", "debugfull"
string (TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS
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

# building QADS or Xlsx requires Qt${QT_VERSION_MAJOR}GuiPrivate (QADS is required, Xlsx is optional)
find_package(Qt${QT_MAJOR_VERSION}Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
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
	Purpose
	SyntaxHighlighting
)

# TODO: make NewStuff optional?
IF (KF${KF_MAJOR_VERSION}NewStuff_FOUND)
	add_definitions (-DHAVE_KF5_NEW_STUFF)
ELSE ()
	MESSAGE (STATUS "KF NewStuff not found")
ENDIF ()
# TODO: service not used?
IF (NOT KF${KF_MAJOR_VERSION}Service_FOUND)
	MESSAGE (STATUS "KF Service not found")
ENDIF ()
# TODO: parts not used?
IF (NOT KF${KF_MAJOR_VERSION}Parts_FOUND)
	MESSAGE (STATUS "KF Parts not found")
ENDIF ()

if (KF${KF_MAJOR_VERSION}Purpose_FOUND)
	add_definitions (-DHAVE_PURPOSE)
endif()

IF (KF${KF_MAJOR_VERSION}SyntaxHighlighting_FOUND)
	add_definitions (-DHAVE_KF5_SYNTAX_HIGHLIGHTING)
ELSE ()
	MESSAGE (STATUS "KF SyntaxHighlighting not found")
ENDIF ()

if (QT_MAJOR_VERSION GREATER_EQUAL 6)
	find_package(KUserFeedbackQt6)
else ()
	find_package(KUserFeedback)
endif ()
IF (KUserFeedback_FOUND OR KUserFeedbackQt6_FOUND)
	MESSAGE (STATUS "Found KUserFeedback")
	add_definitions (-DHAVE_KUSERFEEDBACK)
ELSE ()
	MESSAGE (STATUS "KUserFeedback not found")
ENDIF ()

find_package(BISON REQUIRED)

### compiler flags ######################################
option (ENABLE_COMPILER_OPTIMIZATION "Optimization: -OX" true)
if (${ENABLE_COMPILER_OPTIMIZATION})
    set(COMPILER_OPTIMIZATION_FLAG "-O2")
else()
    set(COMPILER_OPTIMIZATION_FLAG "-O0")
endif()
set (GENERIC_FLAGS "-Wall -Wextra -Wundef -Wpointer-arith -Wunreachable-code -Wunused -Wdeprecated-declarations -fno-omit-frame-pointer -fstack-protector")
set (GENERIC_GNU_FLAGS "${COMPILER_OPTIMIZATION_FLAG} -Wcast-align -Wswitch-enum -fvisibility=default -pedantic")
set (GENERIC_C_FLAGS "${GENERIC_FLAGS} -fno-exceptions")
# liborigin needs exceptions
set (GENERIC_CXX_FLAGS "${GENERIC_FLAGS} -fexceptions -std=c++17")

if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
	message(STATUS "GNU C compiler ${CMAKE_C_COMPILER_VERSION} detected, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS} ${GENERIC_GNU_FLAGS}")
	if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 10.99 AND CMAKE_C_COMPILER_VERSION VERSION_LESS 12)	# GCC 11 fails building readstat 1.1.8
		message(STATUS "Building ReadStat disabled due to GNU C compiler version 11")
		set(DONT_BUILD_READSTAT TRUE)
	endif ()
elseif ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
	message(STATUS "Clang C compiler ${CMAKE_C_COMPILER_VERSION} detected, adding compile flags")
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
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W3")
	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Od")
	else ()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
	endif ()
	set(MSVC_FOUND TRUE)
else ()
	message(STATUS "UNKNOWN C compiler, adding compile flags")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	message(STATUS "GNU C++ compiler ${CMAKE_CXX_COMPILER_VERSION} detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS} -Wzero-as-null-pointer-constant") # -Wzero-as-null-pointer-constant since version 5
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	message(STATUS "Clang C++ compiler ${CMAKE_CXX_COMPILER_VERSION} detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS} -Wzero-as-null-pointer-constant") # -Wzero-as-null-pointer-constant since version 5
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
	message(STATUS "Intel C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE ${GENERIC_CXX_FLAGS}")
	#-std=c++0x comes with cmake's general flags, deprecated in icc, remove it
	string(REPLACE "-std=c++0x" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
	message(STATUS "PGI C++ compiler detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -O3 -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -Minform=inform -Mbounds -Mchkstk")
# " x" postfix to work around a bug in CMake that causes "MSVC" to translate to something completely different
elseif (("${CMAKE_CXX_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
	message(STATUS "MSVC C++ compiler detected, adding compile flags")
	# -D_ALLOW_KEYWORD_MACROS for "#define private public" in MultiRangeTest.cpp
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3 -DPSAPI_VERSION=1 /Zc:externC-")
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Od")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	endif()
	set(MSVC_FOUND TRUE)
else ()
	message(STATUS "UNKNOWN C++ compiler, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS}")
endif ()

##########################################################

# see also https://wiki.qt.io/Using_QString_Effectively
set(LABPLOT_COMPILE_DEFINITIONS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)
if (NOT WIN32)
    # Strict iterators can't be used on Windows, they lead to a link error
    # when application code iterates over a QVector<QPoint> for instance, unless
    # Qt itself was also built with strict iterators.
    # See example at https://bugreports.qt.io/browse/AUTOSUITE-946
    add_definitions(-DQT_STRICT_ITERATORS)
endif()

include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION=\"${labplot2_VERSION}\")
add_definitions (-DCXX_COMPILER=\"${CMAKE_CXX_COMPILER_ID}\ ${CMAKE_CXX_COMPILER_VERSION}\")
add_definitions (-DCXX_COMPILER_FLAGS=\"${CMAKE_CXX_FLAGS}\")
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
      # -extra-arg=--std=c++17
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
# Cantor does not support Qt6 yet
if (QT_MAJOR_VERSION GREATER_EQUAL 6)
	option(ENABLE_CANTOR "Build with Cantor support" OFF)
else ()
	option(ENABLE_CANTOR "Build with Cantor support" ON)
endif ()
option(ENABLE_FFTW "Build with FFTW support" ON)
option(ENABLE_HDF5 "Build with HDF5 support" ON)
option(ENABLE_NETCDF "Build with NetCDF support" ON)
option(ENABLE_FITS "Build with FITS support" ON)
option(ENABLE_LIBCERF "Build with libcerf support" ON)
option(ENABLE_LIBORIGIN "Build with liborigin support" ON)
option(ENABLE_ROOT "Build with ROOT (CERN) support" ON)
# FreeBSD fails with missing csv.h
if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
	option(ENABLE_READSTAT "Build with ReadStat support" OFF)
else ()
	option(ENABLE_READSTAT "Build with ReadStat support" ON)
endif ()
option(ENABLE_MATIO "Build with Matio support" ON)
option(ENABLE_MQTT "Build with MQTT support" ON)
option(ENABLE_QTSERIALPORT "Build with QtSerialPort support" ON)
option(ENABLE_DISCOUNT "Build with Discount support" ON)
option(ENABLE_XLSX "Build with XLSX (Excel) support" ON)
IF (WIN32)	# C++17 needed by Orcus not working with MSVC
	option(ENABLE_ORCUS "Build with Orcus support" OFF)
ELSE ()
	option(ENABLE_ORCUS "Build with Orcus support" ON)
ENDIF ()
option(ENABLE_VECTOR_BLF "Build with Vector BLF file format support" ON)
option(ENABLE_EIGEN3 "Build with Eigen3 support" ON)
IF (APPLE)
	# touch bar is Qt5 only
	if (QT_MAJOR_VERSION GREATER_EQUAL 6)
		option(ENABLE_TOUCHBAR "Enable touch bar support on macOS" OFF)
	else ()
		option(ENABLE_TOUCHBAR "Enable touch bar support on macOS" ON)
	endif ()
ENDIF ()
option(ENABLE_REPRODUCIBLE "Enable reproducible builds" OFF)
option(ENABLE_TESTS "Build with tests" ON)
option(ENABLE_SDK "Build with SDK support" ON)

### OS macros ####################################
IF (WIN32)
	add_definitions (-DHAVE_WINDOWS)
	find_library (PSAPI Psapi)
	if (PSAPI)
		message (STATUS "Found PSAPI: ${PSAPI}")
	else ()
		message (STATUS "PSAPI not found")
	endif ()
ENDIF ()

### GSL (required) ###############################
FIND_PACKAGE(GSL REQUIRED)

FIND_PACKAGE(Poppler "0.62.0" COMPONENTS Qt${QT_MAJOR_VERSION})
# TODO: support Poppler-Qt6
IF (Poppler_FOUND AND (QT_MAJOR_VERSION LESS 6))
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
		add_definitions (-DCANTOR_DISABLED)
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
	add_definitions (-DHDF5_DISABLED)
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
	add_definitions (-DNETCDF_DISABLED)
	MESSAGE (STATUS "Network Common Data Format (NetCDF) Library DISABLED")
ENDIF ()

### MQTT (optional) ###############################
IF (ENABLE_MQTT)
	find_package(Qt${QT_VERSION_MAJOR}Mqtt ${QT_MIN_VERSION} NO_MODULE)
	SET_PACKAGE_PROPERTIES (Qt${QT_VERSION_MAJOR}Mqtt PROPERTIES
		DESCRIPTION "Qt Module to implement MQTT protocol version 3.1 and 3.1.1"
		URL "https://github.com/qt/qtmqtt")
	IF (Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
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
	find_package(Qt${QT_MAJOR_VERSION}SerialPort ${QT_MIN_VERSION} NO_MODULE)
	SET_PACKAGE_PROPERTIES (Qt{QT_MAJOR_VERSION}SerialPort PROPERTIES
		DESCRIPTION "Qt Serial Port library"
		URL "https://doc.qt.io/qt-5/qtserialport-index.html"
		PURPOSE "Support basic functionality of serial ports.")
	IF (Qt${QT_MAJOR_VERSION}SerialPort_FOUND)
		MESSAGE (STATUS "Found Qt${QT_MAJOR_VERSION}SerialPort Library")
		add_definitions (-DHAVE_QTSERIALPORT)
	ELSE ()
		MESSAGE (STATUS "Qt${QT_MAJOR_VERSION}SerialPort Library NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Qt${QT_MAJOR_VERSION}SerialPort Library DISABLED")
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
	add_definitions (-FITS_DISABLED)
	MESSAGE (STATUS "Flexible Image Transport System Data Format (FITS) Library DISABLED")
ENDIF ()

### LIBCERF (optional) #############################
IF (ENABLE_LIBCERF)
	FIND_PACKAGE (LIBCERF)
	IF (LIBCERF_FOUND)
		add_definitions (-DHAVE_LIBCERF)
		include_directories (${LIBCERF_INCLUDE_DIR})
	ELSE ()
		MESSAGE (STATUS "libcerf library NOT FOUND")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "libcerf library DISABLED")
ENDIF ()

### ZLIB for ROOT and READSTAT #################

FIND_PACKAGE(ZLIB)
SET_PACKAGE_PROPERTIES (ZLIB PROPERTIES
	DESCRIPTION "General purpose compression library"
	URL "https://www.zlib.net/"
)
IF (NOT ZLIB_FOUND)
	SET(ZLIB_LIBRARIES "")
ENDIF ()

### ROOT (optional) #############################
IF (ENABLE_ROOT)
	FIND_PACKAGE(LZ4)
	IF (ZLIB_FOUND AND LZ4_FOUND)
		MESSAGE (STATUS "Found ZIP libraries ZLIB and LZ4 (needed for ROOT importer)")
		add_definitions (-DHAVE_ZIP)
	ELSE ()
		MESSAGE (STATUS "ZIP libraries ZLIB or LZ4 (needed for ROOT importer) NOT FOUND")
	ENDIF ()
ELSE ()
	add_definitions (-DROOT_DISABLED)
	MESSAGE (STATUS "ROOT (CERN) importer DISABLED")
ENDIF ()

### ReadStat (optional) #############################
IF (ENABLE_READSTAT)
	FIND_PACKAGE (ReadStat)
	IF (NOT READSTAT_FOUND AND NOT WIN32 AND NOT DONT_BUILD_READSTAT)	# own version not on Windows and not when forbidden
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
		# in case the target is not installed yet: use source dir
		include_directories (${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
	ENDIF ()
	IF (READSTAT_FOUND OR BUILD_READSTAT)	# found or build
		add_definitions (-DHAVE_READSTAT)
	ELSE ()
		SET(READSTAT_LIBRARIES "")
	ENDIF ()
ELSE ()
	add_definitions (-DREADSTAT_DISABLED)
	MESSAGE (STATUS "ReadStat support DISABLED")
ENDIF ()

### XLSX (Excel) (optional) #############################
IF (ENABLE_XLSX)
        FIND_PACKAGE (QXlsx)
        IF (QXLSX_FOUND)
		MESSAGE (STATUS "QXlsx library FOUND.")
		add_definitions (-DHAVE_QXLSX)
        ELSE ()
		if (TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
			set(BUILD_QXLSX TRUE)

			MESSAGE (STATUS "QXlsx library NOT FOUND. Building own version.")
			add_definitions (-DHAVE_QXLSX)
		else ()
			MESSAGE (STATUS "Missing Qt${QT_VERSION_MAJOR}::GuiPrivate to build own QXlsx.")
		endif ()
        ENDIF ()
ELSE ()
	add_definitions (-DXLSX_DISABLED)
	MESSAGE (STATUS "XLSX support DISABLED")
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
	add_definitions (-DMATIO_DISABLED)
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
		MESSAGE (STATUS "Found Markdown Library Discount ${Discount_VERSION}")
		IF (DEFINED Discount_VERSION AND Discount_VERSION VERSION_GREATER_EQUAL 3)
			add_definitions (-DHAVE_DISCOUNT3)
		ENDIF ()
	ELSE ()
		MESSAGE (STATUS "Discount library NOT FOUND.")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Discount DISABLED")
ENDIF ()

### Orcus (optional) #############################
IF (ENABLE_ORCUS)
	FIND_PACKAGE(Orcus)
	SET_PACKAGE_PROPERTIES (Orcus PROPERTIES
		DESCRIPTION "a library that provides a collection of standalone file processing filters"
		URL "https://gitlab.com/orcus/orcus"
		TYPE OPTIONAL)
	IF (Orcus_FOUND)
		add_definitions (-DHAVE_ORCUS)
		include_directories(${Orcus_INCLUDE_DIR} ${Ixion_INCLUDE_DIR})
		MESSAGE (STATUS "Found Orcus and Ixion: ${Orcus_INCLUDE_DIR} ${Ixion_INCLUDE_DIR}, ${Orcus_LIBRARIES} ${Ixion_LIBRARY}")
	ELSE ()
		MESSAGE (STATUS "Orcus library or Ixion NOT FOUND.")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Orcus (ODS support) DISABLED")
ENDIF ()

### Eigen (optional) #############################
IF (ENABLE_EIGEN3)
	FIND_PACKAGE (Eigen3)
	IF (EIGEN3_FOUND)
		MESSAGE (STATUS "Found Eigen3 library version ${EIGEN3_VERSION_STRING}")
		add_definitions (-DHAVE_EIGEN3)
		include_directories (${EIGEN3_INCLUDE_DIR})
	ELSE ()
		MESSAGE (STATUS "Eigen3 library NOT FOUND.")
	ENDIF ()
ELSE ()
	MESSAGE (STATUS "Eigen3 support DISABLED")
ENDIF ()

### Touch bar on macOS (optional) #############################
IF (APPLE AND ENABLE_TOUCHBAR)
	add_definitions (-DHAVE_TOUCHBAR)
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
ENDIF ()

add_subdirectory(data)
add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(doc)

IF (ENABLE_TESTS)
    enable_testing(true)
    add_subdirectory(tests)
ENDIF ()
IF (ENABLE_SDK)
	message(STATUS "SDK build ENABLED")
	add_subdirectory(lib)
ELSE ()
	message(STATUS "SDK build DISABLED")
ENDIF ()

install(FILES org.kde.labplot2.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# clang format
#file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
#kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

find_package(KF${KF_MAJOR_VERSION}I18n CONFIG REQUIRED)

IF (KF${KF_MAJOR_VERSION}I18n_FOUND)
	ki18n_install(po)
ENDIF()
IF (KF${KF_MAJOR_VERSION}DocTools_FOUND)
	kdoctools_install(po)
ENDIF()
