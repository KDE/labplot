# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_program(CCCC_EXECUTABLE
  NAMES cccc cccc.exe
  HINTS
    "$ENV{ProgramFiles}/CCCC"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\C and C++ Code Counter_is1;Inno Setup: App Path]/bin"
  DOC "C and C++ Code Counter (http://cccc.sourceforge.net)")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCCC DEFAULT_MSG CCCC_EXECUTABLE)

mark_as_advanced(CCCC_EXECUTABLE)
