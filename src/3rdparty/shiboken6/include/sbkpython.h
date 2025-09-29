// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKPYTHON_H
#define SBKPYTHON_H

#include "sbkversion.h"

// PYSIDE-2701: This definition is needed for all Python formats with "#".
#define PY_SSIZE_T_CLEAN

// Qt's "slots" macro collides with the "slots" member variables
// used in some Python structs. For compilers that support push_macro,
// temporarily undefine it.
#if defined(slots) && (defined(__GNUC__) || defined(_MSC_VER) || defined(__clang__))
#  pragma push_macro("slots")
#  undef slots

extern "C" {
#  include <Python.h>
}

#  include <structmember.h>
// Now we have the usual variables from Python.h .
#  include "shibokenmacros.h"
// "pep384impl.h" may nowhere be included but in this file.
#  include "pep384impl.h"
#  pragma pop_macro("slots")

#else

extern "C" {
#  include <Python.h>
}

#  include <structmember.h>
// Now we have the usual variables from Python.h .
#  include "shibokenmacros.h"
// "pep384impl.h" may nowhere be included but in this file.
#  include "pep384impl.h"
#endif

// In Python 3, Py_TPFLAGS_DEFAULT contains Py_TPFLAGS_HAVE_VERSION_TAG,
// which will trigger the attribute cache, which is not intended in Qt for Python.
// Use a customized Py_TPFLAGS_DEFAULT by defining Py_TPFLAGS_HAVE_VERSION_TAG = 0.
#undef Py_TPFLAGS_HAVE_VERSION_TAG
#define Py_TPFLAGS_HAVE_VERSION_TAG  (0)

using SbkObjectType [[deprecated]] = PyTypeObject; // FIXME PYSIDE 7 remove

#endif
