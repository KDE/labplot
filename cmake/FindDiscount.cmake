# - Find Discount
# Find the Discount markdown library.
#
# This module defines
#  Discount_FOUND - whether the Discount library was found
#  Discount_LIBRARIES - the Discount library
#  Discount_INCLUDE_DIR - the include path of the Discount library

# SPDX-FileCopyrightText: 2017 Julian Wolff <wolff@julianwolff.de>
# SPDX-FileCopyrightText: 2018 Sune Vuorela <sune@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (Discount_INCLUDE_DIR AND Discount_LIBRARIES)

  # Already in cache
  set (Discount_FOUND TRUE)

else (Discount_INCLUDE_DIR AND Discount_LIBRARIES)

  find_library (Discount_LIBRARIES
    NAMES markdown libmarkdown
  )

  find_path (Discount_INCLUDE_DIR
    NAMES mkdio.h
  )

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (Discount DEFAULT_MSG Discount_LIBRARIES Discount_INCLUDE_DIR)

endif (Discount_INCLUDE_DIR AND Discount_LIBRARIES)

mark_as_advanced(Discount_INCLUDE_DIR Discount_LIBRARIES)

if (Discount_FOUND)
   add_library(Discount::Lib UNKNOWN IMPORTED)
   set_target_properties(Discount::Lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${Discount_INCLUDE_DIR} IMPORTED_LOCATION ${Discount_LIBRARIES})
endif()
