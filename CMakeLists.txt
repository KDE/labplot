project(labplot2)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)

include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDE4Defaults)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O2 -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wunreachable-code -Wunused -Wswitch-enum -fno-omit-frame-pointer -fstack-protector -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wunreachable-code -Wunused -Wswitch-enum -fno-omit-frame-pointer -fstack-protector -fno-exceptions")

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION=\"2.4.0\")
# add_definitions (-DLDEBUG='1')
set(BUILD_SHARED_LIBS true)
add_definitions(-fvisibility=default)

cmake_policy(SET CMP0002 OLD)
#cmake_policy(SET CMP0063 NEW)

### Options ######################################
option(ENABLE_FFTW "Build with FFTW support" "ON")
option(ENABLE_FITS "Build with FITS support" "ON")
option(ENABLE_HDF5 "Build with HDF5 support" "ON")
option(ENABLE_NETCDF "Build with NetCDF support" "ON")

### GSL (required) ###############################
FIND_LIBRARY(GSL_LIBRARIES gsl
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_LIBRARY (GSL_CBLAS_LIBRARIES gslcblas
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_PATH (GSL_INCLUDE_DIR gsl/gsl_multimin.h
	/usr/include
	/usr/local/include
)
FIND_PROGRAM (GSL_CONFIG gsl-config
	/usr/bin
	/usr/local/bin
)
IF (GSL_CONFIG)
	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--version" OUTPUT_VARIABLE gsl_version)
	SET (GSL_VERSION ${gsl_version} CACHE STRING "GNU Scientific Library Version")
ENDIF ()
IF (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)
	SET (GSL_FOUND TRUE)
ELSE ()
	SET (GSL_FOUND FALSE)
ENDIF ()

IF (GSL_FOUND)
	MESSAGE (STATUS "Found GNU Scientific Library ${GSL_VERSION}: ${GSL_INCLUDE_DIR} ${GSL_LIBRARIES};${GSL_CBLAS_LIBRARIES}")
	add_definitions (-DHAVE_GSL)
	# TODO : set version ?
	add_definitions (-DGSL_VERSION=GSL_VERSION)
ELSE ()
	MESSAGE (FATAL_ERROR "GNU Scientific Library not found.")
ENDIF ()

### FFTW (optional) ##############################
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
	MESSAGE (STATUS "FFTW 3 Library not found.")
ENDIF ()
ENDIF ()

### HDF5 (optional) ##############################
IF (ENABLE_HDF5)
FIND_PACKAGE(HDF5 COMPONENTS C)
IF (HDF5_FOUND)
	MESSAGE (STATUS "Found Hierarchical Data Format (HDF5) Library: ${HDF5_INCLUDE_DIRS} ${HDF5_LIBRARY}")
	add_definitions (-DHAVE_HDF5)
	include_directories (${HDF5_INCLUDE_DIRS})
ELSE ()
	MESSAGE (STATUS "Hierarchical Data Format (HDF5) Library not found.")
ENDIF ()
ENDIF ()

### NETCDF (optional) #############################
IF (ENABLE_NETCDF)
FIND_LIBRARY (NETCDF_LIBRARY netcdf
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_PATH (NETCDF_INCLUDE_DIR netcdf.h
	/usr/include/
	/usr/local/include/
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
	MESSAGE (STATUS "Network Common Data Format (NetCDF) Library not found.")
ENDIF ()
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
	/usr/include/
	/usr/include/cfitsio/
	/usr/local/include/
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
	MESSAGE (STATUS "Flexible Image Transport System Data Format (FITS) Library not found.")
ENDIF ()
ENDIF ()
#################################################

add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(doc)

install(FILES org.kde.labplot2.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
