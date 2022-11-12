# ***************************************************************************
# *                                                                         *
# *   Copyright : (C) 2010 The University of Toronto                        *
# *   email     : netterfield@astro.utoronto.ca                             *
# *                                                                         *
# *   Copyright : (C) 2010 Peter Kümmel                                     *
# *   email     : syntheticpp@gmx.net                                       *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************

if(NOT GETDATA_INCLUDEDIR)

pkg_check_modules(PC_GETDATA QUIET getdata>=0.6.0)

# Apple: install getdata with --prefix /opt/local

# FIXME: GETDATA_INCLUDEDIR AND GETDATA_LIBRARIES are set by pkg_check_modules, but
# GETDATA_LIBRARY_C and GETDATA_LIBRARY_CPP are not.
# Ubuntu: maybe /usr/local/lib/pkgconfig/getdata.pc is not correct
#if(NOT PKGGETDATA_LIBRARIES)
	set(PKGGETDATA_LIBRARIES getdata++ getdata)
	if (UNIX)
		SET(PKGGETDATA_LIBRARIES ${PKGGETDATA_LIBRARIES} m)
	endif()
#endif()

set(GETDATA_INCLUDEDIR GETDATA_INCLUDEDIR-NOTFOUND CACHE STRING "" FORCE)
FIND_PATH(GETDATA_INCLUDEDIR getdata.h
	HINTS
	ENV GETDATA_DIR
	PATH_SUFFIXES include/getdata include
	PATHS ${GETDATA_INCLUDEDIR})

foreach(it ${PKGGETDATA_LIBRARIES})
	set(lib_release lib_release-NOTFOUND CACHE STRING "" FORCE)
	FIND_LIBRARY(lib_release ${it}
		HINTS ENV GETDATA_DIR PATH_SUFFIXES lib
		PATHS ${kst_3rdparty_dir} ${PKGGETDATA_LIBRARY_DIRS})
	list(APPEND GETDATA_LIBRARIES_RELEASE ${lib_release})
	list(APPEND GETDATA_LIBRARIES_BOTH optimized ${lib_release})
	set(lib_debug lib_debug-NOTFOUND CACHE STRING "" FORCE)
	FIND_LIBRARY(lib_debug ${it}d
		HINTS ENV GETDATA_DIR PATH_SUFFIXES lib
		PATHS ${kst_3rdparty_dir} ${PKGGETDATA_LIBRARY_DIRS})
	list(APPEND GETDATA_LIBRARIES_DEBUG ${lib_debug})
	list(APPEND GETDATA_LIBRARIES_BOTH debug ${lib_debug})
endforeach()

if(GETDATA_LIBRARIES_DEBUG AND GETDATA_LIBRARIES_RELEASE)
	set(GETDATA_LIBRARIES ${GETDATA_LIBRARIES_BOTH} CACHE STRING "" FORCE)
else()
	set(GETDATA_LIBRARIES ${GETDATA_LIBRARIES_RELEASE} CACHE STRING "" FORCE)
endif()

endif()

IF(GETDATA_INCLUDEDIR AND GETDATA_LIBRARIES)
        SET(Getdata_FOUND TRUE)
	SET(GETDATA_INCLUDE_DIR ${GETDATA_INCLUDEDIR})
	message(STATUS "Found GetData:")
	message(STATUS "     includes : ${GETDATA_INCLUDEDIR}")
	message(STATUS "     libraries: ${GETDATA_LIBRARIES}")
ELSE()
    MESSAGE(STATUS "Not found: GetData.")
    MESSAGE(STATUS "      If GetData is installed outside the CMake search path,")
    MESSAGE(STATUS "      set the environmental variable GETDATA_DIR to the")
    MESSAGE(STATUS "      GetData install prefix.")
ENDIF()

message(STATUS "")

