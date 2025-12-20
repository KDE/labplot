// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKTYPEFACTORY_H
#define SBKTYPEFACTORY_H

#include "sbkpython.h"

extern "C"
{

// PYSIDE-535: Encapsulation of PyType_FromSpec special-cased for PyPy
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpec(PyType_Spec *);
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpecWithMeta(PyType_Spec *, PyTypeObject *);
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpecWithBases(PyType_Spec *, PyObject *);
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpecBasesMeta(PyType_Spec *, PyObject *, PyTypeObject *);
LIBSHIBOKEN_API PyTypeObject *SbkType_FromSpec_BMDWB(PyType_Spec *spec,
                                                     PyObject *bases,
                                                     PyTypeObject *meta,
                                                     int dictoffset,
                                                     int weaklistoffset,
                                                     PyBufferProcs *bufferprocs);

} //extern "C"

#endif // SBKTYPEFACTORY_H
