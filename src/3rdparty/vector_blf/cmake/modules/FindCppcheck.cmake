# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_program(CPPCHECK_EXECUTABLE
  NAMES cppcheck cppcheck.exe
  HINTS
    "$ENV{ProgramFiles}/Cppcheck"
  DOC "Cppcheck (http://cppcheck.sourceforge.net)")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cppcheck DEFAULT_MSG CPPCHECK_EXECUTABLE)

mark_as_advanced(CPPCHECK_EXECUTABLE)
