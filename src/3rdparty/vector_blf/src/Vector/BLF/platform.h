// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * This file contains compiler/linker flags applicable for the library and components using it.
 */

#pragma once

#include <Vector/BLF/config.h>

/* GCC */
#ifdef __GNUC__

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#endif

/* Visual Studio */
#ifdef _MSC_VER

/* '...': conversion from '...' to '...', possible loss of data */
#pragma warning (disable: 4244)

/* '...' : class '...' needs to have dll-interface to be used by clients of class '..' */
#pragma warning (disable: 4251)

/* non dll-interface class '...' used as base for dll-interface class */
#pragma warning (disable: 4275)

/* elements of array '...' will be default initialized */
#pragma warning (disable: 4351)

/* nonstandard extension used: enum '...' used in qualified name */
#pragma warning (disable: 4482)

#endif
