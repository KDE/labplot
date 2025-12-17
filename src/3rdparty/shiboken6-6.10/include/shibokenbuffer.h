// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SHIBOKEN_BUFFER_H
#define SHIBOKEN_BUFFER_H

#include "sbkpython.h"
#include "shibokenmacros.h"

namespace Shiboken::Buffer
{
    enum Type {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    /**
     * Creates a new Python buffer pointing to a contiguous memory block at
     * \p memory of size \p size.
     */
    LIBSHIBOKEN_API PyObject *newObject(void *memory, Py_ssize_t size, Type type);

    /**
     * Creates a new <b>read only</b> Python buffer pointing to a contiguous memory block at
     * \p memory of size \p size.
     */
    LIBSHIBOKEN_API PyObject *newObject(const void *memory, Py_ssize_t size);

    /**
     * Check if is ok to use \p pyObj as argument in all function under Shiboken::Buffer namespace.
     */
    LIBSHIBOKEN_API bool checkType(PyObject *pyObj);

    /**
     * Returns a pointer to the memory pointed by the buffer \p pyObj, \p size is filled with the buffer
     * size if not null.
     *
     * If the \p pyObj is a non-contiguous buffer a Python error is set.
     */
    LIBSHIBOKEN_API void *getPointer(PyObject *pyObj, Py_ssize_t *size = nullptr);

    /**
     * Returns a copy of the buffer data which should be free'd.
     *
     * If the \p pyObj is a non-contiguous buffer a Python error is set.
     * nullptr is returned for empty buffers.
     */
    LIBSHIBOKEN_API void *copyData(PyObject *pyObj, Py_ssize_t *size = nullptr);

} // namespace Shiboken::Buffer

#endif
