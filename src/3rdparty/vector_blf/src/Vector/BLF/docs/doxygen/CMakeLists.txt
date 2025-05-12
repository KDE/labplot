# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

if(WIN32)
    set(DOXYGEN_GENERATE_HTMLHELP YES)
    set(DOXYGEN_GENERATE_TREEVIEW NO)
    set(DOXYGEN_GENERATE_MAN NO)
else()
    set(DOXYGEN_GENERATE_HTMLHELP NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_MAN YES)
endif()

configure_file(Doxyfile.in
    Doxyfile @ONLY IMMEDIATE)

add_custom_target(doxygen ALL
    ${DOXYGEN_EXECUTABLE} Doxyfile > doxygen-out.txt 2> doxygen-err.txt
    SOURCES Doxyfile)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/doxygen)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/doxygen-out.txt
        ${CMAKE_CURRENT_BINARY_DIR}/doxygen-err.txt
        ${CMAKE_CURRENT_BINARY_DIR}/doxygen.tag
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/doxygen)

if(UNIX)
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man
        DESTINATION ${CMAKE_INSTALL_MANDIR}/..)
endif()
