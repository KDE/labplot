# Locate the shiboken6 executable and library.
#
# Variables provided:
#   Shiboken6_FOUND           - TRUE if shiboken6 executable and library were found
#   Shiboken6_PATH            - Path to the shiboken6 Python package directory
#   Shiboken6_INCLUDE_DIRS    - Shiboken6 header location
#   Shiboken6_EXECUTABLE      - Full path to shiboken6 executable
#   Shiboken6_LIBRARY         - Full path to libshiboken6 library

if(NOT Shiboken6_FOUND)
	execute_process(
		COMMAND ${Python3_EXECUTABLE} -c "import shiboken6; print(shiboken6.__path__[0])"
		OUTPUT_VARIABLE Shiboken6_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)

	if(Shiboken6_PATH)
		message(STATUS "Found Shiboken6 via Python: ${Shiboken6_PATH}")
	endif()
endif()

if(Shiboken6_PATH)
    set(Shiboken6_FOUND TRUE)
else()
    set(Shiboken6_FOUND FALSE)
endif()

# Find the executable
find_program(Shiboken6_EXECUTABLE
    NAMES shiboken6
    HINTS "${CMAKE_INSTALL_PREFIX}/bin" "${CMAKE_PREFIX_PATH}/bin" /usr/bin /usr/local/bin /app/bin  "${PySide6_PATH}/../shiboken6_generator"
)

# find header
find_path(Shiboken6_INCLUDE_DIRS
    NAMES sbkversion.h
    PATHS "${Shiboken6_PATH}/include" "${PySide6_PATH}/../shiboken6/include" "${PySide6_PATH}/../shiboken6_generator/include" "${CMAKE_INSTALL_PREFIX}/include" "${CMAKE_INSTALL_PREFIX}/shiboken6/include" "${CMAKE_INSTALL_PREFIX}/include/shiboken6" /usr/include/shiboken6 /usr/local/include /usr/local/include/shiboken6 /usr/local/shiboken6/include
)

# Find the library
find_library(Shiboken6_LIBRARY
    NAMES shiboken6 libshiboken6
    HINTS "${CMAKE_INSTALL_PREFIX}/lib" "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_INSTALL_PREFIX}/lib64" "${CMAKE_PREFIX_PATH}/lib64"
)

find_library(Shiboken6_ABI3_LIBRARY
    NAMES shiboken6.abi3 libshiboken6.abi3 libshiboken6.abi3.so.6.10 libshiboken6.abi3.6.10 libshiboken6.abi3.6.10.dylib libshiboken6.abi3.so.6.9 libshiboken6.abi3.6.9 libshiboken6.abi3.6.9.dylib
    PATHS "${Shiboken6_PATH}" "${PySide6_PATH}/../shiboken6" /usr/lib64 /usr/lib /app/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shiboken6
	REQUIRED_VARS Shiboken6_PATH
)

if(Shiboken6_FOUND)
    # Dummy INTERFACE target for libshiboken
    if(NOT TARGET Shiboken6::libshiboken)
        add_library(Shiboken6::libshiboken INTERFACE IMPORTED)
        if(Shiboken6_LIBRARY)
            set_target_properties(Shiboken6::libshiboken PROPERTIES
                IMPORTED_LOCATION "${Shiboken6_LIBRARY}"
            )
        endif()
        if(Shiboken6_ABI3_LIBRARY)
            set_target_properties(Shiboken6::libshiboken PROPERTIES
                IMPORTED_LOCATION "${Shiboken6_ABI3_LIBRARY}"
            )
        endif()
        if(Shiboken6_INCLUDE_DIRS)
            set_target_properties(Shiboken6::libshiboken PROPERTIES
               INTERFACE_INCLUDE_DIRECTORIES ${Shiboken6_INCLUDE_DIRS}
            )
        endif()
    endif()
endif()

