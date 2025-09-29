# Locate a PySide6 Python installation.
#
# Variables provided:
#   PySide6_FOUND          - TRUE if PySide6 was found
#   PySide6_PYTHONPATH     - Path to the PySide6 Python package directory
#
# Imported targets:
#   PySide6::PythonModule  - INTERFACE target pointing at PySide6 .pyi files

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(NOT DEFINED PYSIDE_PYTHONPATH)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c
                "import PySide6, pathlib; print(pathlib.Path(PySide6.__file__).parent)"
        OUTPUT_VARIABLE PYSIDE_PYTHONPATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(EXISTS "${PYSIDE_PYTHONPATH}")
    set(PySide6_FOUND TRUE)
    set(PySide6_PYTHONPATH "${PYSIDE_PYTHONPATH}" CACHE PATH "PySide6 Python module path")
else()
    set(PySide6_FOUND FALSE)
endif()

find_library(PySide6_ABI3_LIBRARY
    NAMES pyside6.abi3 libpyside6.abi3 libpyside6.abi3.so.6.9
    PATHS "${PYSIDE_PYTHONPATH}" /usr/lib64 /usr/lib /app/lib
)

find_path(PySide6_INCLUDE_DIR
    NAMES PySide6
    PATHS /usr/include /usr/local/include ${PYSIDE_PYTHONPATH}/include ${CMAKE_INSTALL_PREFIX}/include
)

find_path(PySide6_TYPESYSTEMS
   NAMES typesystem_widgets.xml
   PATHS ${PYSIDE_PYTHONPATH}/typesystems ${CMAKE_INSTALL_PREFIX}/PySide6/typesystems /usr/share/PySide6/typesystems
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PySide6 REQUIRED_VARS PySide6_PYTHONPATH)

if(PySide6_FOUND)
    # INTERFACE target for Python modules
    if(NOT TARGET PySide6::PythonModule)
        add_library(PySide6::PythonModule INTERFACE IMPORTED)
        target_include_directories(PySide6::PythonModule INTERFACE "${PySide6_PYTHONPATH}")
    endif()

    # Create a "PySide6::pyside6" target as a dummy INTERFACE
    if(NOT TARGET PySide6::pyside6)
        add_library(PySide6::pyside6 INTERFACE IMPORTED)
        set_target_properties(PySide6::pyside6 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PySide6_PYTHONPATH}"
        )
	if(PySide6_INCLUDE_DIR)
	    set_target_properties(PySide6::pyside6 PROPERTIES
		    INTERFACE_INCLUDE_DIRECTORIES ${PySide6_INCLUDE_DIR}/PySide6
            )
	endif()
    endif()

    # Optionally, alias PySide6::PythonModule to PySide6::pyside6
    #add_library(PySide6::PythonModule ALIAS PySide6::pyside6)
endif()
