project(labplot2)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)

include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDE4Defaults)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wunreachable-code -fno-omit-frame-pointer -fstack-protector -fno-exceptions")

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION=\"2.1.0\")
# add_definitions (-DLDEBUG='1')
set(BUILD_SHARED_LIBS true)
add_definitions(-fvisibility=default)

cmake_policy(SET CMP0002 OLD)
cmake_policy(SET CMP0063 NEW)

### GSL ######################################
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
FIND_PATH (GSL_INCLUDE_DIR gsl_multimin.h
	/usr/include/gsl
	/usr/local/include/gsl
)
FIND_PROGRAM (GSL_CONFIG gsl-config
	/usr/bin
	/usr/local/bin
)
IF (GSL_CONFIG)
	EXEC_PROGRAM (${GSL_CONFIG} ARGS "--version" OUTPUT_VARIABLE gsl_version)
	SET (GSL_VERSION ${gsl_version} CACHE STRING "GNU Scientific Library Version")
ENDIF (GSL_CONFIG)
IF (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)
	SET (GSL_FOUND TRUE)
ELSE (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)
	SET (GSL_FOUND FALSE)
ENDIF (GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES AND GSL_INCLUDE_DIR)

IF (GSL_FOUND)
	MESSAGE (STATUS "Found GNU Scientific Library ${GSL_VERSION}: ${GSL_INCLUDE_DIR} ${GSL_LIBRARIES};${GSL_CBLAS_LIBRARIES}")
	add_definitions (-DHAVE_GSL)
	# TODO : set version ?
	add_definitions (-DGSL_VERSION=GSL_VERSION)
ELSE (GSL_FOUND)
	MESSAGE (FATAL_ERROR "GNU Scientific Library not found.")
ENDIF (GSL_FOUND)

### HDF5 #####################################
FIND_LIBRARY(HDF5_LIBRARY hdf5
	PATHS
	/usr/lib
	/usr/local/lib
)
FIND_PATH (HDF5_INCLUDE_DIR hdf5.h
	/usr/include/
	/usr/local/include/
)
IF (HDF5_LIBRARY AND HDF5_INCLUDE_DIR)
	SET (HDF5_FOUND TRUE)
ELSE (HDF5_LIBRARY AND HDF5_INCLUDE_DIR)
	SET (HDF5_FOUND FALSE)
	SET (HDF5_LIBRARY "")
ENDIF (HDF5_LIBRARY AND HDF5_INCLUDE_DIR)
IF (HDF5_FOUND)
	MESSAGE (STATUS "Found Hierarchical Data Format (HDF) 5 Library: ${HDF5_INCLUDE_DIR} ${HDF5_LIBRARY}")
	add_definitions (-DHAVE_HDF5)
ELSE (HDF5_FOUND)
	MESSAGE (STATUS "Hierarchical Data Format (HDF) 5 Library not found.")
ENDIF (HDF5_FOUND)

# ### NETCDF #####################################
FIND_LIBRARY(NETCDF_LIBRARY netcdf
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
ELSE (NETCDF_LIBRARY AND NETCDF_INCLUDE_DIR)
	SET (NETCDF_FOUND FALSE)
	SET (NETCDF_LIBRARY "")
ENDIF (NETCDF_LIBRARY AND NETCDF_INCLUDE_DIR)
IF (NETCDF_FOUND)
	MESSAGE (STATUS "Found Network Common Data Format (NetCDF) Library: ${NETCDF_INCLUDE_DIR} ${NETCDF_LIBRARY}")
	add_definitions (-DHAVE_NETCDF)
ELSE (NETCDF_FOUND)
	MESSAGE (STATUS "Network Common Data Format (NetCDF) Library not found.")
ENDIF (NETCDF_FOUND)

add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(doc)

install(FILES labplot.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)
