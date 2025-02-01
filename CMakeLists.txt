# see https://community.kde.org/Policies/CMake_Coding_Style
cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)
project(labplot VERSION 2.11.80 LANGUAGES CXX C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
message(STATUS "Found Qt ${QT_VERSION}")
set(QT_MAJOR_VERSION ${QT_VERSION_MAJOR})

set(QT_MIN_VERSION "6.5.0")
add_definitions(-DQT_DISABLE_DEPRECATED_UP_TO=0x50F00)

set(KF_MAJOR_VERSION "6")
set(KF_MIN_VERSION "5.240.0")

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
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS
    Concurrent
    Gui
#     Qml
#     Quick
#     QuickWidgets
    PrintSupport
    Sql
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
    I18n
    IconThemes
    KIO
    TextWidgets
    WidgetsAddons
    XmlGui
    NewStuffCore
    NewStuff
OPTIONAL_COMPONENTS
    DocTools
    Service
    Parts
    Purpose
    SyntaxHighlighting
)

# TODO: make NewStuff optional?
if(KF${KF_MAJOR_VERSION}NewStuff_FOUND)
    add_definitions(-DHAVE_KF_NEW_STUFF)
else()
    message(STATUS "KF NewStuff not found")
endif()
# TODO: service not used?
if(NOT KF${KF_MAJOR_VERSION}Service_FOUND)
    message(STATUS "KF Service not found")
endif()
# TODO: parts not used?
if(NOT KF${KF_MAJOR_VERSION}Parts_FOUND)
    message(STATUS "KF Parts not found")
endif()

if(KF${KF_MAJOR_VERSION}Purpose_FOUND)
    add_definitions(-DHAVE_PURPOSE)
endif()

if(KF${KF_MAJOR_VERSION}SyntaxHighlighting_FOUND)
    add_definitions(-DHAVE_KF_SYNTAX_HIGHLIGHTING)
else()
    message(STATUS "KF SyntaxHighlighting not found")
endif()

find_package(KF${KF_MAJOR_VERSION}UserFeedback)
if(KF${KF_MAJOR_VERSION}UserFeedback_FOUND)
    message(STATUS "Found KF${KF_MAJOR_VERSION}UserFeedback")
    add_definitions(-DHAVE_KUSERFEEDBACK)
else()
    message(STATUS "KF${KF_MAJOR_VERSION}UserFeedback not found")
endif()

find_package(BISON REQUIRED)

### compiler flags ######################################
option (ENABLE_COMPILER_OPTIMIZATION "Optimization: -OX" true)
if(${ENABLE_COMPILER_OPTIMIZATION})
    set(COMPILER_OPTIMIZATION_FLAG "-O2")
else()
    set(COMPILER_OPTIMIZATION_FLAG "-O0")
endif()
set (GENERIC_FLAGS "-Wall -Wextra -Wundef -Wpointer-arith -Wunreachable-code -Wunused -Wdeprecated-declarations -fno-omit-frame-pointer -fstack-protector-strong")
#set (GENERIC_GNU_FLAGS "${COMPILER_OPTIMIZATION_FLAG} -Wcast-align -Wswitch-enum -fvisibility=default -pedantic")
set (GENERIC_GNU_FLAGS "${COMPILER_OPTIMIZATION_FLAG} -Wcast-align -Wswitch-enum -fvisibility=default -pedantic -ffunction-sections -fdata-sections -pipe")
if(APPLE)
    set (GENERIC_GNU_LINK_FLAGS "-Wl,--no-undefined")
else()
    set (GENERIC_GNU_LINK_FLAGS "-Wl,--gc-sections -Wl,--as-needed -Wl,--no-undefined")
endif()
set (GENERIC_C_FLAGS "${GENERIC_FLAGS} -fno-exceptions")
# liborigin needs exceptions
set (GENERIC_CXX_FLAGS "${GENERIC_FLAGS} -fexceptions -std=c++17")

if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    message(STATUS "GNU C compiler ${CMAKE_C_COMPILER_VERSION} detected, adding compile flags")
    # -flto is too heavy even with TMPDIR=/dev/shm
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS} ${GENERIC_GNU_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GENERIC_GNU_LINK_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GENERIC_GNU_LINK_FLAGS}")
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 10.99 AND CMAKE_C_COMPILER_VERSION VERSION_LESS 12)	# GCC 11 fails building readstat 1.1.8
        message(STATUS "Building ReadStat disabled due to GNU C compiler version 11")
        set(DONT_BUILD_READSTAT TRUE)
    endif()
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14) # overwrite -Werror=hardened to not fail
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=hardened")
    endif()
elseif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "Clang C compiler ${CMAKE_C_COMPILER_VERSION} detected, adding compile flags")
    # even -flto=thin takes too long
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE ${GENERIC_C_FLAGS} ${GENERIC_GNU_FLAGS}")
    # TRY: -flto=thin -flto-jobs=2
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GENERIC_GNU_LINK_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GENERIC_GNU_LINK_FLAGS}")
elseif("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
    message(STATUS "Intel C compiler detected, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -O3 ${GENERIC_C_FLAGS}")
elseif("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")
    message(STATUS "PGI C compiler detected, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -O3 -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -Minform=inform -Mbounds -Mchkstk")
# " x" postfix to work around a bug in CMake that causes "MSVC" to translate to something completely different
elseif(("${CMAKE_C_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
    message(STATUS "MSVC C compiler detected, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W3")
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Od")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    endif()
    set(MSVC_FOUND TRUE)
else()
    message(STATUS "UNKNOWN C compiler, adding compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GENERIC_C_FLAGS}")
endif()
message(STATUS "CFLAGS =${CMAKE_C_FLAGS}")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    if(QT_VERSION VERSION_GREATER_EQUAL 6.7.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
	# see Qt supported compiler
        message(FATAL_ERROR "Insufficient GNU C++ version ${CMAKE_CXX_COMPILER_VERSION} (Qt 6.7+ requires at least GNU g++ 9)")
    else()
        message(STATUS "GNU C++ compiler ${CMAKE_CXX_COMPILER_VERSION} detected, adding compile flags")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS} -Wzero-as-null-pointer-constant")
	# see linker flags of gcc
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14) # overwrite -Werror=hardened to not fail
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=hardened")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "Clang C++ compiler ${CMAKE_CXX_COMPILER_VERSION} detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE ${GENERIC_CXX_FLAGS} ${GENERIC_GNU_FLAGS} -Wzero-as-null-pointer-constant")
    # see linker flags of clang
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    message(STATUS "Intel C++ compiler detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE ${GENERIC_CXX_FLAGS}")
    #-std=c++0x comes with cmake's general flags, deprecated in icc, remove it
    string(REPLACE "-std=c++0x" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    message(STATUS "PGI C++ compiler detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -O3 -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -Minform=inform -Mbounds -Mchkstk")
# " x" postfix to work around a bug in CMake that causes "MSVC" to translate to something completely different
elseif(("${CMAKE_CXX_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
    message(STATUS "MSVC C++ compiler detected, adding compile flags")
    # -D_ALLOW_KEYWORD_MACROS for "#define private public" in MultiRangeTest.cpp
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3 -DPSAPI_VERSION=1 /Zc:externC-")
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Od")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    endif()
    set(MSVC_FOUND TRUE)
else()
    message(STATUS "UNKNOWN C++ compiler, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GENERIC_CXX_FLAGS}")
endif()
message(STATUS "CXXFLAGS =${CMAKE_CXX_FLAGS}")

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
if(NOT WIN32)
    # Strict iterators can't be used on Windows, they lead to a link error
    # when application code iterates over a QVector<QPoint> for instance, unless
    # Qt itself was also built with strict iterators.
    # See example at https://bugreports.qt.io/browse/AUTOSUITE-946
    add_definitions(-DQT_STRICT_ITERATORS)
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions(-DLVERSION=\"${labplot_VERSION}\")
add_definitions(-DCXX_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\")
add_definitions(-DCXX_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\")
add_definitions(-DCXX_COMPILER_FLAGS=\"${CMAKE_CXX_FLAGS}\")
set(BUILD_SHARED_LIBS true)

#cmake_policy(SET CMP0002 OLD)
if(CMAKE_VERSION VERSION_EQUAL "3.3" OR CMAKE_VERSION VERSION_GREATER "3.3")
    cmake_policy(SET CMP0063 NEW)
endif()

if(CMAKE_VERSION VERSION_GREATER "3.5")
  set(ENABLE_CLANG_TIDY OFF CACHE BOOL "Add clang-tidy automatically to builds")
  if(ENABLE_CLANG_TIDY)
    find_program (CLANG_TIDY_EXE NAMES "clang-tidy" PATHS /usr/bin)
    if(CLANG_TIDY_EXE)
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
add_definitions(-DGIT_COMMIT=\"${GIT_COMMIT}\")


### Options ######################################
option(ENABLE_CANTOR "Build with Cantor support" ON)
option(ENABLE_FFTW "Build with FFTW support" ON)
option(ENABLE_HDF5 "Build with HDF5 support" ON)
option(ENABLE_NETCDF "Build with NetCDF support" ON)
option(ENABLE_FITS "Build with FITS support" ON)
option(ENABLE_LIBCERF "Build with libcerf support" ON)
option(ENABLE_LIBORIGIN "Build with liborigin support" ON)
option(ENABLE_ROOT "Build with ROOT (CERN) support" ON)
# FreeBSD fails with missing csv.h
if("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    option(ENABLE_READSTAT "Build with ReadStat support" OFF)
else()
    option(ENABLE_READSTAT "Build with ReadStat support" ON)
endif()
option(ENABLE_MATIO "Build with Matio support" ON)
option(ENABLE_MQTT "Build with MQTT support" ON)
option(ENABLE_QTSERIALPORT "Build with QtSerialPort support" ON)
option(ENABLE_QTSVG "Build with QtSvg support" ON)
option(ENABLE_DISCOUNT "Build with Discount support" ON)
option(ENABLE_XLSX "Build with XLSX (Excel) support" ON)
option(ENABLE_MCAP "Build with MCAP support" ON)
option(ENABLE_ORCUS "Build with Orcus support" ON)
option(ENABLE_VECTOR_BLF "Build with Vector BLF file format support" ON)
option(ENABLE_EIGEN3 "Build with Eigen3 support" ON)
if(APPLE)
    # touch bar is Qt5 only
    if(QT_MAJOR_VERSION GREATER_EQUAL 6)
        option(ENABLE_TOUCHBAR "Enable touch bar support on macOS" OFF)
    else()
        option(ENABLE_TOUCHBAR "Enable touch bar support on macOS" ON)
    endif()
endif()
option(ENABLE_REPRODUCIBLE "Enable reproducible builds" OFF)
option(ENABLE_TESTS "Build with tests" ON)
option(ENABLE_SDK "Build with SDK support" ON)
if(ENABLE_SDK)
    option(ENABLE_SDK_EXAMPLES "Build SDK examples" OFF)
endif()

### OS macros ####################################
if(WIN32)
    add_definitions(-DHAVE_WINDOWS)
    find_library (PSAPI Psapi)
    if(PSAPI)
        message (STATUS "Found PSAPI: ${PSAPI}")
    else()
        message (STATUS "PSAPI not found")
    endif()
endif()

### GSL (required) ###############################
find_package(GSL REQUIRED)
message(STATUS "GSL libraries: ${GSL_LIBRARIES}, GSL library: ${GSL_LIBRARY}, GSL CBLAS library: ${GSL_CBLAS_LIBRARY}")

find_package(Poppler "0.62.0" COMPONENTS Qt${QT_MAJOR_VERSION})
if(Poppler_FOUND)
    include_directories(${Poppler_INCLUDE_DIRS})
    message(STATUS "Poppler libraries: ${Poppler_LIBRARIES} (version \"${Poppler_VERSION}\")")
    add_definitions(-DHAVE_POPPLER)
endif()

### liborigin (included) ###############################
if(ENABLE_LIBORIGIN)
    find_package(LibOrigin)
    if(NOT LIBORIGIN_FOUND)    # use own version
        if(CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "debugfull")
            message(STATUS "Origin project import (through internal liborigin) enabled (parser logging enabled)")
            set(ENABLE_ORIGIN_PARSER_LOG TRUE)
        else()
            message(STATUS "Origin project import (through internal liborigin) enabled (parser logging disabled)")
        endif()
    endif()

    add_definitions(-DHAVE_LIBORIGIN)
else()
    message(STATUS "Origin project import DISABLED")
endif()

### Cantorlibs (optional) ###############################
if(ENABLE_CANTOR)
    find_package(Cantor "24.11.70")
    if(Cantor_FOUND)
        message(STATUS "Found Cantor: ${Cantor_LIBRARIES}, ${Cantor_INCLUDE_DIR} (version \"${Cantor_VERSION}\")")
        add_definitions(-DHAVE_CANTOR_LIBS)
	add_definitions(-DCANTOR_VERSION_STRING=\"${Cantor_VERSION}\")
    else()
        message(STATUS "Cantor Library NOT FOUND")
    endif()
else()
    add_definitions(-DCANTOR_DISABLED)
    message(STATUS "Cantor Library DISABLED")
endif()

### FFTW (optional) #####################################
if(ENABLE_FFTW)
    find_package(FFTW3)
    if(FFTW3_FOUND)
        add_definitions(-DHAVE_FFTW3)
	add_definitions(-DFFTW3_VERSION_STRING=\"${FFTW3_VERSION}\")
    else()
        message(STATUS "FFTW 3 Library NOT FOUND")
    endif()
else()
    message(STATUS "FFTW 3 Library DISABLED")
endif()

### HDF5 (optional) ##############################
if(ENABLE_HDF5)
    find_package(HDF5 COMPONENTS C)
    set_package_properties(HDF5 PROPERTIES
        DESCRIPTION "Reading and writing self describing array data"
        URL "https://www.hdfgroup.org/solutions/hdf5/"
    )
    if(HDF5_FOUND)
        add_definitions(-DHAVE_HDF5)
        if(MSVC_FOUND)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DH5_BUILT_AS_DYNAMIC_LIB")
        endif()
        if(HDF5_VERSION VERSION_GREATER "1.9")
            add_definitions(-DHAVE_AT_LEAST_HDF5_1_10_0)
        endif()
        if(HDF5_VERSION VERSION_GREATER "1.10.0.1")
            add_definitions(-DHAVE_AT_LEAST_HDF5_1_10_0)
            add_definitions(-DHAVE_AT_LEAST_HDF5_1_10_1)
        endif()
        include_directories(${HDF5_INCLUDE_DIRS})
    else()
        message(STATUS "Hierarchical Data Format (HDF5) Library NOT FOUND")
        set(HDF5_LIBRARIES "")
    endif()
else()
    add_definitions(-DHDF5_DISABLED)
    message(STATUS "Hierarchical Data Format (HDF5) Library DISABLED")
endif()

### NETCDF (optional) #############################
if(ENABLE_NETCDF)
    find_package(netCDF)
    SET_PACKAGE_PROPERTIES(netCDF PROPERTIES
        DESCRIPTION "Interfaces for array-oriented data access"
        URL "https://www.unidata.ucar.edu/software/netcdf/"
    )
    if(netCDF_FOUND)
        add_definitions(-DHAVE_NETCDF)
        # netCDF on Windows may link to zip.dll
        find_library(Zip_LIBRARIES NAMES zip)
        if(Zip_LIBRARIES)
            message(STATUS "Found Zip Library: ${Zip_LIBRARIES}")
        endif()
    else()
        message(STATUS "Network Common Data Format (NetCDF) Library NOT FOUND")
    endif()
else()
    add_definitions(-DNETCDF_DISABLED)
    message(STATUS "Network Common Data Format (NetCDF) Library DISABLED")
endif()

### MQTT (optional) ###############################
if(ENABLE_MQTT)
    find_package(Qt${QT_VERSION_MAJOR}Mqtt ${QT_MIN_VERSION} NO_MODULE)
    set_package_properties(Qt${QT_VERSION_MAJOR}Mqtt PROPERTIES
        DESCRIPTION "Qt Module to implement MQTT protocol version 3.1 and 3.1.1"
        URL "https://github.com/qt/qtmqtt")
    if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND)
        message(STATUS "Found MQTT Library")
        add_definitions(-DHAVE_MQTT)
    else()
        message(STATUS "MQTT Library NOT FOUND")
    endif()
else()
    message(STATUS "MQTT Library DISABLED")
endif()

### QtSerialPort (optional) ###############################
if(ENABLE_QTSERIALPORT)
    find_package(Qt${QT_MAJOR_VERSION}SerialPort ${QT_MIN_VERSION} NO_MODULE)
    set_package_properties(Qt{QT_MAJOR_VERSION}SerialPort PROPERTIES
        DESCRIPTION "Qt Serial Port library"
        URL "https://doc.qt.io/qt-6/qtserialport-index.html"
        PURPOSE "Support basic functionality of serial ports.")
    if(Qt${QT_MAJOR_VERSION}SerialPort_FOUND)
        message(STATUS "Found Qt${QT_MAJOR_VERSION}SerialPort Library")
        add_definitions(-DHAVE_QTSERIALPORT)
    else()
        message(STATUS "Qt${QT_MAJOR_VERSION}SerialPort Library NOT FOUND")
    endif()
else()
    message(STATUS "Qt${QT_MAJOR_VERSION}SerialPort Library DISABLED")
endif()

### QtSvg (optional) ###############################
if(ENABLE_QTSVG)
    find_package(Qt${QT_MAJOR_VERSION}Svg ${QT_MIN_VERSION} NO_MODULE)
    set_package_properties(Qt{QT_MAJOR_VERSION}Svg PROPERTIES
        DESCRIPTION "Qt SVG library"
        URL "https://doc.qt.io/qt-6/qtsvg-index.html"
        PURPOSE "Scalable Vector Graphics support.")
    if(Qt${QT_MAJOR_VERSION}Svg_FOUND)
        message(STATUS "Found Qt${QT_MAJOR_VERSION}Svg Library")
        add_definitions(-DHAVE_QTSVG)
    else()
        message(STATUS "Qt${QT_MAJOR_VERSION}Svg Library NOT FOUND")
    endif()
else()
    message(STATUS "Qt${QT_MAJOR_VERSION}Svg Library DISABLED")
endif()

### FITS (optional) ###############################
if(ENABLE_FITS)
    find_package(CFitsio)
    set_package_properties(CFitsio PROPERTIES
        DESCRIPTION "FITS IO Library"
        URL "https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html"
        PURPOSE "Support for the FITS (Flexible Image Transport System) data format.")
    if(CFITSIO_FOUND)
        add_definitions(-DHAVE_FITS)
	add_definitions(-DCFITSIO_VERSION_STRING=\"${CFITSIO_VERSION}\")
        include_directories(${CFITSIO_INCLUDE_DIR})
    else()
        message(STATUS "Flexible Image Transport System Data Format (FITS) Library NOT FOUND")
    endif()
else()
    add_definitions(-FITS_DISABLED)
    message(STATUS "Flexible Image Transport System Data Format (FITS) Library DISABLED")
endif()

### LIBCERF (optional) #############################
if(ENABLE_LIBCERF)
    find_package(LIBCERF)
    if(LIBCERF_FOUND)
        add_definitions(-DHAVE_LIBCERF)
	add_definitions(-DLIBCERF_VERSION_STRING=\"${LIBCERF_VERSION}\")
        include_directories(${LIBCERF_INCLUDE_DIR})
    else()
        message(STATUS "libcerf library NOT FOUND")
    endif()
else()
    message(STATUS "libcerf library DISABLED")
endif()

### ZLIB for ROOT and READSTAT #################
find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "General purpose compression library"
    URL "https://www.zlib.net/"
)
if(NOT ZLIB_FOUND)
    set(ZLIB_LIBRARIES "")
endif()

### LZ4 for ROOT and MCAP #################
if(ENABLE_ROOT OR ENABLE_MCAP)
    find_package(LZ4)
    IF(LZ4_FOUND)
	#message(STATUS "Found LZ4: ${LZ4_LIBRARIES}, ${LZ4_INCLUDE_DIR} (found version \"${LZ4_VERSION}\")")
        add_definitions(-DHAVE_LZ4)
    endif()
endif()
### ZSTD for MCAP #################

option(LOCAL_MCAP_DOWNLOAD "Download MCAP" OFF)
IF(ENABLE_MCAP)
    if(LOCAL_MCAP_DOWNLOAD)
        message(STATUS "Downloading MCAP")
    else()
        message(STATUS "Using included MCAP")
    endif()

    add_definitions(-DHAVE_MCAP)
    find_package(ZSTD)
    if(ZSTD_FOUND)
        message(STATUS "Found ZSTD (used for MCAP ZSTD support)")
        add_definitions(-DHAVE_ZSTD)
    else()
        message(STATUS "Could not find ZSTD (needed for MCAP ZSTD support)")
    endif()
else()
    message(STATUS "MCAP DISABLED")
endif()

### ROOT (optional) #############################
if(ENABLE_ROOT)
    if(ZLIB_FOUND AND LZ4_FOUND)
        message(STATUS "Found ZIP libraries ZLIB and LZ4 (needed for ROOT importer)")
        add_definitions(-DHAVE_ZIP)
    else()
        message(STATUS "ZIP libraries ZLIB or LZ4 (needed for ROOT importer) NOT FOUND")
    endif()
else()
    add_definitions(-DROOT_DISABLED)
    message(STATUS "ROOT (CERN) importer DISABLED")
endif()

### ReadStat (optional) #############################
if(ENABLE_READSTAT)
    find_package(ReadStat)
    if(NOT READSTAT_FOUND AND NOT WIN32 AND NOT DONT_BUILD_READSTAT)    # own version not on Windows and not when forbidden
        message(STATUS "ReadStat library NOT FOUND. Building own version")
        set(BUILD_READSTAT TRUE)

        # link own readstat with iconv
        find_library(ICONV_LIBRARIES NAMES iconv libiconv libiconv-2)
        if(ICONV_LIBRARIES) # non-glibc
            message(STATUS "Iconv library FOUND: ${ICONV_LIBRARIES}")
            set(READSTAT_LIBRARIES ${ICONV_LIBRARIES} ${CMAKE_BINARY_DIR}/src/3rdparty/install/lib/libreadstat.a)
        else()
            message(STATUS "Iconv library NOT FOUND")
            set(READSTAT_LIBRARIES ${CMAKE_BINARY_DIR}/src/3rdparty/install/lib/libreadstat.a)
        endif()
        include_directories(${CMAKE_BINARY_DIR}/src/3rdparty/install/include)
        # in case the target is not installed yet: use source dir
	if(EXISTS ${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
            include_directories(${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
        endif()
    endif()
    if(READSTAT_FOUND OR BUILD_READSTAT)    # found or build
        add_definitions(-DHAVE_READSTAT)
    else()
        set(READSTAT_LIBRARIES "")
    endif()
else()
    add_definitions(-DREADSTAT_DISABLED)
    message(STATUS "ReadStat support DISABLED")
endif()

### XLSX (Excel) (optional) #############################
if(ENABLE_XLSX)
        find_package(QXlsx)
        if(QXLSX_FOUND)
        message(STATUS "QXlsx library FOUND.")
        add_definitions(-DHAVE_QXLSX)
        else()
        if(TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
            set(BUILD_QXLSX TRUE)

            message(STATUS "QXlsx library NOT FOUND. Building own version.")
            add_definitions(-DHAVE_QXLSX)
        else()
            message(STATUS "Missing Qt${QT_VERSION_MAJOR}::GuiPrivate to build own QXlsx.")
        endif()
        endif()
else()
    add_definitions(-DXLSX_DISABLED)
    message(STATUS "XLSX support DISABLED")
endif()

###  Matio (optional) ##############################################
if(ENABLE_MATIO)
    find_package(Matio)
    if(MATIO_FOUND)
        add_definitions(-DHAVE_MATIO)
    else()
        message(STATUS "Matio library NOT FOUND.")
    endif()
else()
    add_definitions(-DMATIO_DISABLED)
    message(STATUS "Matio support DISABLED")
endif()

### Discount (optional) #############################
if(ENABLE_DISCOUNT)
    find_package(Discount)
    set_package_properties(Discount PROPERTIES
        DESCRIPTION "A C implementation of the Markdown markup language"
        URL "https://www.pell.portland.or.us/~orc/Code/discount/"
        TYPE OPTIONAL)
    if(Discount_FOUND)
        add_definitions(-DHAVE_DISCOUNT)
	message(STATUS "Found Markdown Library Discount (version \"${Discount_VERSION}\")")
	add_definitions(-DDISCOUNT_VERSION_STRING=\"${Discount_VERSION}\")
        if(DEFINED Discount_VERSION AND Discount_VERSION VERSION_GREATER_EQUAL 3)
            add_definitions(-DHAVE_DISCOUNT3)
        endif()
    else()
        message(STATUS "Discount library NOT FOUND.")
    endif()
else()
    message(STATUS "Discount DISABLED")
endif()

### Orcus (optional) #############################
if(ENABLE_ORCUS)
    find_package(Orcus)
    set_package_properties(Orcus PROPERTIES
        DESCRIPTION "a library that provides a collection of standalone file processing filters"
        URL "https://gitlab.com/orcus/orcus"
        TYPE OPTIONAL)
    if(Orcus_FOUND)
        add_definitions(-DHAVE_ORCUS)
	add_definitions(-DORCUS_VERSION_STRING=\"${LIBORCUS_VERSION}\")
        include_directories(${Orcus_INCLUDE_DIR} ${Ixion_INCLUDE_DIR})
	message(STATUS "Found Orcus version ${LIBORCUS_VERSION} and Ixion ${LIBIXION_VERSION}: ${Orcus_INCLUDE_DIR} ${Ixion_INCLUDE_DIR}, ${Orcus_LIBRARIES} ${Ixion_LIBRARY}")
    else()
        message(STATUS "Orcus library or Ixion NOT FOUND.")
    endif()
else()
    message(STATUS "Orcus (ODS support) DISABLED")
endif()

### Eigen (optional) #############################
if(ENABLE_EIGEN3)
    find_package(Eigen3)
    if(EIGEN3_FOUND)
	    message(STATUS "Found Eigen3 library: ${EIGEN3_INCLUDE_DIR} (version \"${EIGEN3_VERSION_STRING}\")")
        add_definitions(-DHAVE_EIGEN3)
	add_definitions(-DEIGEN3_VERSION_STRING=\"${EIGEN3_VERSION_STRING}\")
        include_directories(${EIGEN3_INCLUDE_DIR})
    else()
        message(STATUS "Eigen3 library NOT FOUND.")
    endif()
else()
    message(STATUS "Eigen3 support DISABLED")
endif()

# Qt Advanced Docking System (required) #####################
find_package(qt${QT_VERSION_MAJOR}advanceddocking CONFIG)
if(NOT qt${QT_VERSION_MAJOR}advanceddocking_FOUND)
    option(LOCAL_QT_ADVANCED_DOCK_DOWNLOAD "Use local build of qt advanced dock downloaded directly from the git repository" OFF)
    option(LOCAL_QT_ADVANCED_DOCK_COPY "Use local build of qt advanced dock copied into the Qt-Advanced-Docking-System subfolder" ON)
    if(TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
        message(STATUS "Building local version of Qt-Advanced-Docking-System.")
    else ()
        message(FATAL_ERROR "Missing Qt${QT_VERSION_MAJOR}::GuiPrivate to build local version of Qt-Advanced-Docking-System.")
    endif()
endif()

### Touch bar on macOS (optional) #############################
if(APPLE AND ENABLE_TOUCHBAR)
    add_definitions(-DHAVE_TOUCHBAR)
endif()

#################################################
if(ENABLE_REPRODUCIBLE)
    add_definitions(-DREPRODUCIBLE_BUILD)
    message(STATUS "Reproducable build ENABLED")
else()
    message(STATUS "Reproducable build DISABLED")
endif()
#################################################
#################################################
include(CheckFunctionExists)

CHECK_FUNCTION_EXISTS(random HAVE_RANDOM_FUNCTION)
#################################################
find_path(XLOCALE_INCLUDE_DIR xlocale.h
    /usr/include
    /usr/local/include
)
if(XLOCALE_INCLUDE_DIR)
    add_definitions(-DHAVE_XLOCALE)
    include_directories(${XLOCALE_INCLUDE_DIR})
endif()

add_subdirectory(data)
add_subdirectory(icons)
add_subdirectory(src)

if(KF${KF_MAJOR_VERSION}DocTools_FOUND)
    add_subdirectory(doc)
endif()

if(ENABLE_TESTS)
    enable_testing(true)
    add_subdirectory(tests)
endif()
if(ENABLE_SDK)
    message(STATUS "SDK build ENABLED")
    add_subdirectory(lib)
else()
    message(STATUS "SDK build DISABLED")
endif()

install(FILES org.kde.labplot.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# clang format
#file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
#kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

get_target_property(COMPILE_DEFINITIONS labplot COMPILE_DEFINITIONS)
message(STATUS "COMPILE_DEFINITIONS = ${COMPILE_DEFINITIONS}")
get_directory_property(DIRECTORY_DEFINITIONS COMPILE_DEFINITIONS)
message(STATUS "DIRECTORY DEFINITIONS = ${DIRECTORY_DEFINITIONS}\n")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

find_package(KF${KF_MAJOR_VERSION}I18n CONFIG REQUIRED)

if(KF${KF_MAJOR_VERSION}I18n_FOUND)
    ki18n_install(po)
endif()
if(KF${KF_MAJOR_VERSION}DocTools_FOUND)
    kdoctools_install(po)
endif()
