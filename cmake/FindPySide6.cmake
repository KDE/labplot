# Locate a PySide6 Python installation.
#
# Variables provided:
#   PySide6_FOUND          - TRUE if PySide6 was found
#   PySide6_PATH           - Path to the PySide6 Python package directory
#   PySide6_ABI3_LIBRARY   - PySide6 library path
#   PySide6_INCLUDE_DIRS   - PySide6 header location
#   Pyside6_TYPESYSTEMS    - PySide6 typesystem location
#
# Imported targets:
#   PySide6::PythonModule  - INTERFACE target pointing at PySide6 .pyi files

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# find PySide6 via Python
if(NOT DEFINED PySide6_PATH)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import PySide6; print(PySide6.__path__[0])"
	OUTPUT_VARIABLE PySide6_PYTHONPATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _pyside6_import_result
    )

    if(PySide6_PYTHONPATH)
	    message(STATUS "Found PySide6 via Python: ${PySide6_PYTHONPATH}")
    elseif(_pyside6_import_result)
        # The Python interpreter may not have PySide6 in its default sys.path
        # (e.g. Craft on macOS where the Development and Interpreter components
        # resolve to different Python installations). Search for PySide6 in
        # site-packages directories relative to the Python library.
        set(_pyside6_search_bases "")
        if(Python3_LIBRARY_DIRS)
            list(APPEND _pyside6_search_bases ${Python3_LIBRARY_DIRS})
            # On macOS with framework builds, Python3_LIBRARY_DIRS points inside
            # the framework (e.g. .../lib/Python.framework/Versions/3.11/lib)
            # but PySide6 may be installed in the outer lib dir (e.g. .../lib/).
            foreach(_libdir IN LISTS Python3_LIBRARY_DIRS)
                string(FIND "${_libdir}" "/Python.framework/" _fw_pos)
                if(NOT _fw_pos EQUAL -1)
                    string(SUBSTRING "${_libdir}" 0 ${_fw_pos} _outer_lib)
                    list(APPEND _pyside6_search_bases "${_outer_lib}")
                endif()
            endforeach()
        endif()
        # Derive from include dir: .../include/python3.X -> .../lib
        if(Python3_INCLUDE_DIRS)
            foreach(_inc IN LISTS Python3_INCLUDE_DIRS)
                get_filename_component(_prefix "${_inc}" DIRECTORY)
                get_filename_component(_prefix "${_prefix}" DIRECTORY)
                list(APPEND _pyside6_search_bases "${_prefix}/lib")
            endforeach()
        endif()
        # Derive from interpreter path
        if(Python3_EXECUTABLE)
            get_filename_component(_prefix "${Python3_EXECUTABLE}" DIRECTORY)
            get_filename_component(_prefix "${_prefix}" DIRECTORY)
            list(APPEND _pyside6_search_bases "${_prefix}/lib")
        endif()
        list(REMOVE_DUPLICATES _pyside6_search_bases)

        foreach(_base IN LISTS _pyside6_search_bases)
            file(GLOB _pyside6_candidates
                "${_base}/python*/site-packages/PySide6"
                "${_base}/Python.framework/Versions/*/lib/python*/site-packages/PySide6"
            )
            if(_pyside6_candidates)
                list(GET _pyside6_candidates 0 _pyside6_candidate)
                if(EXISTS "${_pyside6_candidate}")
                    set(PySide6_PYTHONPATH "${_pyside6_candidate}")
                    message(STATUS "Found PySide6 via path search: ${PySide6_PYTHONPATH}")
                    break()
                endif()
            endif()
        endforeach()
    endif()
endif()

if(EXISTS "${PySide6_PYTHONPATH}")
    set(PySide6_FOUND TRUE)
    set(PySide6_PATH "${PySide6_PYTHONPATH}" CACHE PATH "PySide6 Python module path")
else()
    set(PySide6_FOUND FALSE)
endif()

find_path(PySide6_INCLUDE_DIRS
    NAMES pysideqobject.h
    PATHS ${PySide6_PATH}/include ${CMAKE_INSTALL_PREFIX}/include ${CMAKE_INSTALL_PREFIX}/include/PySide6 /usr/include/PySide6 /usr/local/include /usr/local/include/PySide6 /usr/local/PySide6/include
)

find_path(PySide6_TYPESYSTEMS
    NAMES typesystem_widgets.xml
    PATHS ${PySide6_PATH}/typesystems ${CMAKE_INSTALL_PREFIX}/PySide6/typesystems /usr/share/PySide6/typesystems /usr/local/share/PySide6/typesystems
)

find_library(PySide6_ABI3_LIBRARY
    NAMES pyside6.abi3 libpyside6.abi3 libpyside6.abi3.so.6.10 libpyside6.abi3.6.10 libpyside6.abi3.6.10.dylib libpyside6.abi3.so.6.9 libpyside6.abi3.6.9 libpyside6.abi3.6.9.dylib
    PATHS "${PySide6_PATH}" /usr/lib64 /usr/lib /app/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PySide6 REQUIRED_VARS PySide6_PATH)

if(PySide6_FOUND)
    # INTERFACE target for Python modules
    if(NOT TARGET PySide6::PythonModule)
        add_library(PySide6::PythonModule INTERFACE IMPORTED)
	target_include_directories(PySide6::PythonModule INTERFACE "${PySide6_PATH}")
    endif()

    # Create a "PySide6::pyside6" target as a dummy INTERFACE
    if(NOT TARGET PySide6::pyside6)
        add_library(PySide6::pyside6 INTERFACE IMPORTED)
        set_target_properties(PySide6::pyside6 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PySide6_PATH}"
        )
        if(PySide6_INCLUDE_DIRS)
            set_target_properties(PySide6::pyside6 PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${PySide6_INCLUDE_DIRS}
           )
        endif()
   endif()
endif()
