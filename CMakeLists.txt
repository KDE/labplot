project(LabPlot2)
cmake_minimum_required(VERSION 2.6) 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wunreachable-code -fno-omit-frame-pointer -fstack-protector -fno-exceptions")

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_definitions (-DLVERSION='\"2.0.2\"')
# add_definitions (-DLDEBUG='1')
# needed for qt_plugin_instance_scidavis_standardcurvesymbolfactory()
add_definitions(-D QT_STATICPLUGIN )
add_definitions(-DSUPPRESS_SCRIPTING_INIT)
set(BUILD_SHARED_LIBS true)
#set(_GNU_SOURCE true)
add_definitions(-fvisibility=default)


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
	add_definitions (-DHAVE_GSL=1)
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
	add_definitions (-DHAVE_HDF5=1)
ELSE (HDF5_FOUND)
	MESSAGE (STATUS "Hierarchical Data Format (HDF) 5 Library not found.")
ENDIF (HDF5_FOUND)

# ### NETCDF #####################################
# FIND_LIBRARY(NETCDF_LIBRARY
#   NAMES netcdf
# )
# IF(NETCDF_LIBRARY)
# MESSAGE ("-- Found NETCDF library: ${NETCDF_LIBRARY}")
# add_definitions (-DHAVE_NETCDF=1)
# ELSE(NETCDF_LIBRARY)
# MESSAGE ("-- NETCDF library NOT FOUND!")
# SET(NETCDF_LIBRARY "")
# ENDIF(NETCDF_LIBRARY)
#  
# ### CDF #####################################
# FIND_LIBRARY(CDF_LIBRARY
#   NAMES cdf
# )
# IF(CDF_LIBRARY)
# MESSAGE ("-- Found CDF library: ${CDF_LIBRARY}")
# add_definitions (-DHAVE_CDF=1)
# ELSE(CDF_LIBRARY)
# MESSAGE ("-- CDF library NOT FOUND!")
# SET(CDF_LIBRARY "")
# ENDIF(CDF_LIBRARY)
# 
# ### OPJ #####################################
# FIND_LIBRARY(OPJ_LIBRARY
#   NAMES origin
# )
# IF(OPJ_LIBRARY)
# MESSAGE ("-- Found liborigin library: ${OPJ_LIBRARY}")
# ELSE(OPJ_LIBRARY)
# MESSAGE ("-- System OPJ library not found! Using included version.")
# add_subdirectory(liborigin)
# SET(OPJ_LIBRARY "-llabplot_origin")
# # for FilterOPJ to find OPJFile.h
# INCLUDE_DIRECTORIES(liborigin)
# ENDIF(OPJ_LIBRARY)
# ##############################################
add_subdirectory(icons)
add_subdirectory(src)
add_subdirectory(doc)

install(FILES labplot.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)
