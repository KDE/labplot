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

``HOMEPAGE_URL`` is a URL to the project homepage.

``ISSUES_URL` is a URL where users can report bugs and feature requests.

``AUTHOR`` is a string with the author of the library.

``README`` is a Markdown file that will be used as the project's
description on the Python Package Index.

#]=======================================================================]

set(MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_shiboken_sources)
    set(options EXPORT_TYPESYSTEM)
    set(oneValueArgs PACKAGE_NAME WRAPPED_HEADER TYPESYSTEM VERSION QT_VERSION HOMEPAGE_URL ISSUES_URL AUTHOR README)
    set(multiValueArgs GENERATED_SOURCES DEPENDENCIES INCLUDES)

    cmake_parse_arguments(PB "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

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

    # Shiboken cannot resolve CMake generator expressions or nonexistent paths,
    # used before constructing:
    # --include-paths=...
    # --force-process-system-include-paths=...
    # for the flatpak build, the PySide6 include path is not available at build time
    # and need to add it to the force process system include paths.
    # Removing it would reintroduce invalid paths into Shiboken’s command line.
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

    filter_existing_include_dirs(PB_EXISTING_INCLUDES ${PB_INCLUDES})
    set(PB_INCLUDES ${PB_EXISTING_INCLUDES})

    foreach(_dependency ${PB_DEPENDENCIES})
        get_property(DEPENDENCY_INCLUDE_DIRS TARGET "${_dependency}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

        filter_existing_include_dirs(DEPENDENCY_EXISTING_INCLUDE_DIRS ${DEPENDENCY_INCLUDE_DIRS})
        append_shiboken_include_options(DEPENDENCY_EXISTING_INCLUDE_DIRS)
    endforeach()

    append_shiboken_include_options(PB_INCLUDES)

    make_path(FORCE_PROCESS_INCLUDE_PATHS ${FORCE_PROCESS_INCLUDE_DIRS})

    # Set up the options to pass to shiboken.
    set(shiboken_options --enable-pyside-extensions
        --keywords=scripting
        --clang-option=-DSCRIPTING
        ${INCLUDES}
        --force-process-system-include-paths=${FORCE_PROCESS_INCLUDE_PATHS}
        --typesystem-paths="${CMAKE_INSTALL_PREFIX}/share/PySide${QT_MAJOR_VERSION}/typesystems"
	--typesystem-paths="${PySide6_TYPESYSTEMS}"
        --output-directory=${CMAKE_CURRENT_BINARY_DIR})

    set(generated_sources_dependencies ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM})

    # Add custom target to run shiboken to generate the binding cpp files.
    add_custom_command(
        OUTPUT ${PB_GENERATED_SOURCES}
        COMMAND ${Shiboken6_EXECUTABLE} ${shiboken_options} ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM}
        DEPENDS ${generated_sources_dependencies}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "GenerateShibokenSources: Running generator \"${Shiboken6_EXECUTABLE}\" for ${PB_TYPESYSTEM}"
    )

    set_source_files_properties(
	    ${PB_GENERATED_SOURCES}
	    PROPERTIES
	        GENERATED TRUE
		COMPILE_FLAGS
            "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wno-keyword-macro -Wno-shadow -Wno-cast-function-type -Wno-zero-as-null-pointer-constant>"
    )

    # # Set the cpp files which will be used for the bindings library.
    # set(${PB_PACKAGE_NAME}_sources ${PB_GENERATED_SOURCES})

    # # PySide6 uses deprecated code.
    # get_property(_defs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    # list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
    # set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
    # get_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
    # list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
    # set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})

    # # Define and build the bindings library.
    # add_library(${PB_PACKAGE_NAME} SHARED ${${PB_PACKAGE_NAME}_sources})

    # target_link_libraries(${PB_PACKAGE_NAME} PRIVATE
    #     PySide6::pyside6
    #     Shiboken6::libshiboken
    #     ${Python3_LIBRARIES}
    # )

    # # Apply relevant include and link flags.
    # target_include_directories(${PB_PACKAGE_NAME} PRIVATE
    #     ${PYSIDE_PYTHONPATH}/include
    #     ${SHIBOKEN_PYTHON_INCLUDE_DIRS}
    #     $<TARGET_PROPERTY:PySide6::pyside6,INTERFACE_INCLUDE_DIRECTORIES>
    #     $<TARGET_PROPERTY:Shiboken6::libshiboken,INTERFACE_INCLUDE_DIRECTORIES>
    # )

    # # Hide noisy warnings
    # target_compile_options(${PB_PACKAGE_NAME} PRIVATE -Wno-cast-function-type -Wno-missing-include-dirs)

    # # Adjust the name of generated module.
    # set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY PREFIX "")
    # set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY LIBRARY_OUTPUT_NAME "${PB_PACKAGE_NAME}.${Python3_SOABI}")
    # set_property(TARGET ${PB_PACKAGE_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/build/lib)

    # # Build Python Wheel
    # file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/${PB_PACKAGE_NAME}")
    # configure_file("${MODULES_DIR}/ECMGeneratePythonBindings.toml.in" "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/pyproject.toml")
    # configure_file(${PB_README} "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/README.md" COPYONLY)

    # add_custom_command(
    #     TARGET ${PB_PACKAGE_NAME}
    #     POST_BUILD
    #     COMMAND Python3::Interpreter -m build --wheel --no-isolation
    #     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}"
    #     COMMENT "Building Python Wheel"
    # )

    # # Export the header and the typesystem XML file
    # if (PB_EXPORT_TYPESYSTEM)
    #     string(TOLOWER ${PB_PACKAGE_NAME} lower_package_name)
    #     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}/${lower_package_name}_python.h
    #             DESTINATION "${PYSIDE_INCLUDE_DIR}/${PB_PACKAGE_NAME}/")
    #     install(FILES "${PB_TYPESYSTEM}" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/PySide${QT_MAJOR_VERSION}/typesystems/")
    # endif()

endfunction()
