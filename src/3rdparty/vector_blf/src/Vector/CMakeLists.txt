# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_subdirectory(BLF)

if (NOT FETCH_CONTENT_INCLUSION)
    install(
        FILES BLF.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Vector)
endif()
