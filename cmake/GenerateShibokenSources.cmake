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

    # Ugly hacks because PySide6::pyside6 only includes /usr/includes/PySide6 and none of the sub directory
    # Qt bugreport: PYSIDE-2882
    get_property(PYSIDE_INCLUDE_DIRS TARGET "PySide6::pyside6" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT PYSIDE_INCLUDE_DIR)
        set(PYSIDE_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/PySide${QT_MAJOR_VERSION}")
    endif()
    if(NOT PYSIDE_INCLUDE_DIR IN_LIST PYSIDE_INCLUDE_DIRS)
        list(APPEND PYSIDE_INCLUDE_DIRS "${PYSIDE_INCLUDE_DIR}")
    endif()
    foreach(PYSIDE_INCLUDE_DIR ${PYSIDE_INCLUDE_DIRS})
        file(GLOB PYSIDE_SUBDIRS LIST_DIRECTORIES true "${PYSIDE_INCLUDE_DIR}/*")
        foreach (PYSIDE_SUBDIR ${PYSIDE_SUBDIRS})
            if (IS_DIRECTORY ${PYSIDE_SUBDIR})
                set_property(TARGET PySide6::pyside6
                    APPEND
                    PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                    ${PYSIDE_SUBDIR}
                )
            endif()
        endforeach()
    endforeach()

    list(APPEND PB_DEPENDENCIES PySide6::pyside6)
    list(APPEND PB_DEPENDENCIES Shiboken6::libshiboken)

    # Get the relevant include dirs, to pass them on to shiboken.
    set(INCLUDES "")

    if(WIN32)
        set(PATH_SEP "\;")
    else()
        set(PATH_SEP ":")
    endif()

    macro(make_path varname)
        # accepts any number of path variables
        string(REPLACE ";" "${PATH_SEP}" ${varname} "${ARGN}")
    endmacro()

    foreach(_dependency ${PB_DEPENDENCIES})
        get_property(DEPENDENCY_INCLUDE_DIRS TARGET "${_dependency}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

        make_path(_include_dirs $<JOIN:$<TARGET_PROPERTY:${_dependency},INTERFACE_INCLUDE_DIRECTORIES>,${PATH_SEP}>)
        list(APPEND INCLUDES "--include-paths=${_include_dirs}")
    endforeach()

    make_path(PBC_INCLUDES ${PB_INCLUDES})

    # Set up the options to pass to shiboken.
    set(shiboken_options --enable-pyside-extensions
        --keywords=scripting
        --clang-option=-DSCRIPTING
        "${INCLUDES}${PATH_SEP}${PBC_INCLUDES}"
        --include-paths=${CMAKE_SOURCE_DIR}
        --typesystem-paths=${CMAKE_SOURCE_DIR}
        --typesystem-paths="${CMAKE_INSTALL_PREFIX}/share/PySide${QT_MAJOR_VERSION}/typesystems"
        --typesystem-paths=${PYSIDE_TYPESYSTEMS}
	--typesystem-paths=${PySide6_TYPESYSTEMS}
        --output-directory=${CMAKE_CURRENT_BINARY_DIR})

    set(generated_sources_dependencies ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM})

    # Add custom target to run shiboken to generate the binding cpp files.
    add_custom_command(
        OUTPUT ${PB_GENERATED_SOURCES}
        COMMAND shiboken6 ${shiboken_options} ${PB_WRAPPED_HEADER} ${PB_TYPESYSTEM}
        DEPENDS ${generated_sources_dependencies}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running generator for ${PB_TYPESYSTEM}"
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
