// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SHIBOKENMACROS_H
#define SHIBOKENMACROS_H

// LIBSHIBOKEN_API macro is used for the public API symbols.
#if defined _WIN32
#  define LIBSHIBOKEN_EXPORT __declspec(dllexport)
#  ifdef _MSC_VER
#    define LIBSHIBOKEN_IMPORT __declspec(dllimport)
#  else
#    define LIBSHIBOKEN_IMPORT
#  endif
#else
#  define LIBSHIBOKEN_EXPORT __attribute__ ((visibility("default")))
#  define LIBSHIBOKEN_IMPORT
#endif

#ifdef BUILD_LIBSHIBOKEN
#  define LIBSHIBOKEN_API LIBSHIBOKEN_EXPORT
#else
#  define LIBSHIBOKEN_API LIBSHIBOKEN_IMPORT
#endif

#endif // SHIBOKENMACROS_H
