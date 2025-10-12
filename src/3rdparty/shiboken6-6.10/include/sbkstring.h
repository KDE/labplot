// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKSTRING_H
#define SBKSTRING_H

#include "sbkpython.h"
#include "shibokenmacros.h"

namespace Shiboken::String
{
    LIBSHIBOKEN_API bool check(PyObject *obj);
    LIBSHIBOKEN_API bool checkIterable(PyObject *obj);
    /// Check for iterable function arguments (excluding enumerations)
    LIBSHIBOKEN_API bool checkIterableArgument(PyObject *obj);
    LIBSHIBOKEN_API bool checkPath(PyObject *path);
    LIBSHIBOKEN_API bool checkType(PyTypeObject *obj);
    LIBSHIBOKEN_API bool checkChar(PyObject *obj);
    LIBSHIBOKEN_API bool isConvertible(PyObject *obj);
    LIBSHIBOKEN_API PyObject *fromCString(const char *value);
    LIBSHIBOKEN_API PyObject *fromCString(const char *value, int len);
    LIBSHIBOKEN_API const char *toCString(PyObject *str);
    LIBSHIBOKEN_API const char *toCString(PyObject *str, Py_ssize_t *len);
    LIBSHIBOKEN_API bool concat(PyObject **val1, PyObject *val2);
    LIBSHIBOKEN_API PyObject *fromFormat(const char *format, ...);
    LIBSHIBOKEN_API PyObject *fromStringAndSize(const char *str, Py_ssize_t size);
    LIBSHIBOKEN_API int compare(PyObject *val1, const char *val2);
    LIBSHIBOKEN_API Py_ssize_t len(PyObject *str);
    LIBSHIBOKEN_API PyObject *createStaticString(const char *str);
    LIBSHIBOKEN_API PyObject *getSnakeCaseName(const char *name, bool lower);
    LIBSHIBOKEN_API PyObject *getSnakeCaseName(PyObject *name, bool lower);
    LIBSHIBOKEN_API PyObject *repr(PyObject *o);

} // namespace Shiboken::String

#endif
