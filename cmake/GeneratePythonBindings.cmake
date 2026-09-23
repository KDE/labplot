# SPDX-FileCopyrightText: 2023 The Qt Company Ltd.
# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# Based on https://code.qt.io/cgit/pyside/pyside-setup.git/tree/examples/widgetbinding/CMakeLists.txt

#[=======================================================================[.rst:
GeneratePythonBindings
-------------------------

This module is experimental and internal.  The interface will likely
change in the coming releases.

Generate Python bindings using Shiboken.

::

  generate_python_bindings(PACKAGE_NAME <pythonlibrary>
                               VERSION <version>
                               WRAPPED_HEADER <filename>
                               TYPESYSTEM <filename>
                               [EXPORT_TYPESYSTEM]
                               GENERATED_SOURCES <filename> [<filename> [...]]
                               DEPENDENCIES <target> [<target> [...]]
                               QT_VERSION <version>
                               HOMEPAGE_URL <url>
                               ISSUES_URL <url>
                               AUTHOR <string>
                               README <filename> )

``<pythonlibrary>`` is the name of the Python library that will be created.

``VERSION`` is the version of the library.

``WRAPPED_HEADER`` is a C++ header that contains all the required includes
for the library.

``TYPESYSTEM`` is the XML file where the bindings are defined.

``EXPORT_TYPESYSTEM`` specifies that the typesystem XML file and the
generated header are exported and can be used by other typesystem XML files.

``GENERATED_SOURCES`` is the list of generated C++ source files by Shiboken
that will be used to build the shared library.

``QT_VERSION`` is the minimum required Qt version of the library.

``DEPENDENCIES`` is the list of dependencies that the bindings uses.

``HOMEPAGE_URL`` is a URL to the proyect homepage.

``ISSUES_URL` is a URL where users can report bugs and feature requests.

``AUTHOR`` is a string with the author of the library.

``README`` is a Markdown file that will be used as the project's
description on the Python Package Index.

#]=======================================================================]

set(MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_python_bindings)
    set(options EXPORT_TYPESYSTEM)
    set(oneValueArgs PACKAGE_NAME WRAPPED_HEADER TYPESYSTEM VERSION QT_VERSION HOMEPAGE_URL ISSUES_URL AUTHOR README)
    set(multiValueArgs GENERATED_SOURCES DEPENDENCIES INCLUDES)

    cmake_parse_arguments(PB "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

    if (NOT Python3_EXECUTABLE)
        message(FATAL_ERROR "Python3_EXECUTABLE not set. Make sure find_package(Python3) is called before including GeneratePythonBindings")
    endif()

    execute_process(COMMAND ${Python3_EXECUTABLE} -Esc "import build" RESULT_VARIABLE PYTHON_BUILD_CHECK_EXIT_CODE OUTPUT_QUIET ERROR_QUIET)

    if (PYTHON_BUILD_CHECK_EXIT_CODE)
        message(FATAL_ERROR "The 'build' Python module is needed for GeneratePythonBindings")
    endif()

    # PySide6 headers are available from the package include root. Qt module
    # headers are provided separately through the Qt target include directories.
    get_property(PYSIDE_INCLUDE_DIRS TARGET "PySide6::pyside6" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    set(PYSIDE_FALLBACK_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/PySide${QT_MAJOR_VERSION}")
    if(EXISTS "${PYSIDE_FALLBACK_INCLUDE_DIR}" AND NOT PYSIDE_FALLBACK_INCLUDE_DIR IN_LIST PYSIDE_INCLUDE_DIRS)
        list(APPEND PYSIDE_INCLUDE_DIRS "${PYSIDE_FALLBACK_INCLUDE_DIR}")
    endif()
    set_property(TARGET PySide6::pyside6 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PYSIDE_INCLUDE_DIRS}")

    list(APPEND PB_DEPENDENCIES PySide6::pyside6)

    # Get the relevant include dirs, to pass them on to shiboken.
    set(INCLUDES "")
    set(FORCE_PROCESS_INCLUDE_DIRS "")

    if(WIN32)
        set(PATH_SEP "\;")
    else()
        set(PATH_SEP ":")
    endif()

    macro(make_path varname)
        # accepts any number of path variables
        string(REPLACE ";" "${PATH_SEP}" ${varname} "${ARGN}")
    endmacro()

    macro(filter_existing_include_dirs output_var)
        set(${output_var} "")
        foreach(_include_dir ${ARGN})
            if(_include_dir MATCHES "^\\$<BUILD_INTERFACE:(.*)>$")
                set(_include_dir "${CMAKE_MATCH_1}")
            elseif(_include_dir MATCHES "^\\$<")
                continue()
            endif()

            if(EXISTS "${_include_dir}")
                list(APPEND ${output_var} "${_include_dir}")
            endif()
        endforeach()
    endmacro()

    macro(append_shiboken_include_options include_dirs)
        make_path(_include_paths ${${include_dirs}})
        if(_include_paths)
            list(APPEND INCLUDES "--include-paths=${_include_paths}")
        endif()
        foreach(_include_dir ${${include_dirs}})
            list(APPEND FORCE_PROCESS_INCLUDE_DIRS "${_include_dir}")
        endforeach()
    endmacro()

    foreach(_dependency ${PB_DEPENDENCIES})
        get_property(DEPENDENCY_INCLUDE_DIRS TARGET "${_dependency}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

        filter_existing_include_dirs(DEPENDENCY_EXISTING_INCLUDE_DIRS ${DEPENDENCY_INCLUDE_DIRS})
        append_shiboken_include_options(DEPENDENCY_EXISTING_INCLUDE_DIRS)
    endforeach()

    filter_existing_include_dirs(PB_EXISTING_INCLUDES ${PB_INCLUDES})
    append_shiboken_include_options(PB_EXISTING_INCLUDES)

    make_path(FORCE_PROCESS_INCLUDE_PATHS ${FORCE_PROCESS_INCLUDE_DIRS})

    # Set up the options to pass to shiboken.
    set(shiboken_options --enable-pyside-extensions
        ${INCLUDES}
        --force-process-system-include-paths=${FORCE_PROCESS_INCLUDE_PATHS}
        --typesystem-paths="${CMAKE_INSTALL_PREFIX}/share/PySide${QT_MAJOR_VERSION}/typesystems"
	--typesystem-paths=${PySide6_TYPESYSTEMS}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR})

    set(generated_sources_dependencies ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM})

    # Add custom target to run shiboken to generate the binding cpp files.
    add_custom_command(
        OUTPUT ${PB_GENERATED_SOURCES}
        COMMAND ${Shiboken6_EXECUTABLE} ${shiboken_options} ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM}
        DEPENDS ${generated_sources_dependencies}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running generator for ${PB_TYPESYSTEM}"
    )

    # Set the cpp files which will be used for the bindings library.
    set(${PB_PACKAGE_NAME}_sources ${PB_GENERATED_SOURCES})

    # PySide6 uses deprecated code.
    get_property(_defs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
    get_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
    get_property(_defs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_UP_TO=]])
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
    get_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_UP_TO=]])
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})

    # Define and build the bindings library.
    add_library(${PB_PACKAGE_NAME} SHARED ${${PB_PACKAGE_NAME}_sources})

    target_compile_options(${PB_PACKAGE_NAME} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-keyword-macro -Wno-shadow -Wno-cast-function-type -Wno-zero-as-null-pointer-constant>
    )
    target_link_libraries(${PB_PACKAGE_NAME} PRIVATE
        PySide6::pyside6
        Shiboken6::libshiboken
        ${Python3_LIBRARIES}
    )

    # PySide headers are needed to compile the generated binding sources.
    # Flatpak's PySide BaseApp does not necessarily ship Shiboken development
    # headers alongside the generator and runtime library.
    set(PYSIDE_MODULE_INCLUDE_DIRS "")
    foreach(_pyside_module QtCore QtGui QtWidgets)
        set(_pyside_module_include_dir "${PySide6_INCLUDE_DIRS}/${_pyside_module}")
        if(EXISTS "${_pyside_module_include_dir}")
            list(APPEND PYSIDE_MODULE_INCLUDE_DIRS "${_pyside_module_include_dir}")
        endif()
    endforeach()
    target_include_directories(${PB_PACKAGE_NAME} PRIVATE
	${PySide6_INCLUDE_DIRS}
        ${PYSIDE_MODULE_INCLUDE_DIRS}
    )

    # Adjust the name of generated module.
    set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY PREFIX "")
    set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY LIBRARY_OUTPUT_NAME "${PB_PACKAGE_NAME}.${Python3_SOABI}")
    set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/build/lib)

    # Build Python Wheel
    #file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/${PB_PACKAGE_NAME}")
    #configure_file("${MODULES_DIR}/ECMGeneratePythonBindings.toml.in" "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/pyproject.toml")
    #configure_file(${PB_README} "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/README.md" COPYONLY)

    #add_custom_command(
    #    TARGET ${PB_PACKAGE_NAME}
    #    POST_BUILD
    #    COMMAND Python3::Interpreter -m build --wheel --no-isolation
    #    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}"
    #    COMMENT "Building Python Wheel"
    #)

    # Export the header and the typesystem XML file
    if (PB_EXPORT_TYPESYSTEM)
        string(TOLOWER ${PB_PACKAGE_NAME} lower_package_name)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/${lower_package_name}_python.h
                DESTINATION "${PYSIDE_INCLUDE_DIR}/${PB_PACKAGE_NAME}/")
        install(FILES "${PB_TYPESYSTEM}" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/PySide${QT_MAJOR_VERSION}/typesystems/")
    endif()

endfunction()
