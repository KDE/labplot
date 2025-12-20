// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKPEPBUFFER_H
#define SBKPEPBUFFER_H

#include "bufferprocs_py37.h"

// FIXME: Move back to sbktypefactory.h once Py_LIMITED_API >= 3.11
extern "C"
{
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpec_BMDWB(PyType_Spec *spec,
                                                     PyObject *bases,
                                                     PyTypeObject *meta,
                                                     int dictoffset,
                                                     int weaklistoffset,
                                                     PyBufferProcs *bufferprocs);
} // extern "C"

// FIXME: Move back to helper.h once Py_LIMITED_API >= 3.11
namespace Shiboken
{
struct LIBSHIBOKEN_API debugPyBuffer
{
    explicit debugPyBuffer(const Py_buffer &b);

    const Py_buffer &m_buffer;
};

} // namespace Shiboken

#endif // SBKBUFFER_H
