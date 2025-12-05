# Locate the shiboken6 executable and library.
#
# Variables provided:
#   Shiboken6_FOUND           - TRUE if shiboken6 executable and library were found
#   Shiboken6_EXECUTABLE      - Full path to shiboken6 executable
#   Shiboken6_LIBRARY         - Full path to libshiboken6 library
#
# Imported targets:
#   Shiboken6::Executable     - IMPORTED target for the shiboken6 executable
#   Shiboken6::libshiboken    - IMPORTED library target for libshiboken6

# Find the executable
find_program(Shiboken6_EXECUTABLE
    NAMES shiboken6
    HINTS "${CMAKE_INSTALL_PREFIX}/bin" "${CMAKE_PREFIX_PATH}/bin" /usr/bin /usr/local/bin /app/bin  "${PYSIDE_PYTHONPATH}/../shiboken6_generator"
)

# Find the library
find_library(Shiboken6_LIBRARY
    NAMES shiboken6 libshiboken6
    HINTS "${CMAKE_INSTALL_PREFIX}/lib" "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_INSTALL_PREFIX}/lib64" "${CMAKE_PREFIX_PATH}/lib64"
)

find_library(Shiboken6_ABI3_LIBRARY
    NAMES shiboken6.abi3 libshiboken6.abi3 libshiboken6.abi3.so.6.9 libshiboken6.abi3.6.9 libshiboken6.abi3.6.9.dylib
    PATHS "${PYSIDE_PYTHONPATH}/../shiboken6" /usr/lib64 /usr/lib /app/lib
)

find_path(Shiboken6_INCLUDE_DIR
    NAMES sbkversion.h
    PATHS ${PYSIDE_PYTHONPATH}/../shiboken6/include ${PYSIDE_PYTHONPATH}/../shiboken6_generator/include ${CMAKE_INSTALL_PREFIX}/include ${CMAKE_INSTALL_PREFIX}/include/shiboken6 /usr/include/shiboken6 /usr/local/include/shiboken6
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shiboken6
    REQUIRED_VARS Shiboken6_EXECUTABLE
)

if(Shiboken6_FOUND)
    # Executable target
    if(NOT TARGET Shiboken6::Executable)
        add_executable(Shiboken6::Executable IMPORTED)
        set_target_properties(Shiboken6::Executable PROPERTIES
            IMPORTED_LOCATION "${Shiboken6_EXECUTABLE}"
        )
    endif()

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
	if(Shiboken6_INCLUDE_DIR)
	    set_target_properties(Shiboken6::libshiboken PROPERTIES
		    INTERFACE_INCLUDE_DIRECTORIES ${Shiboken6_INCLUDE_DIR}
            )
	endif()

    endif()

    # Optionally, if you have include path for generated headers, add it:
    #target_include_directories(Shiboken6::libshiboken INTERFACE "/app/lib/python3.12/site-packages/shiboken6/include")
endif()
