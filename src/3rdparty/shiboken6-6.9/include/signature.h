// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "shibokenmacros.h"
#include "sbkpython.h"

extern "C"
{

LIBSHIBOKEN_API int InitSignatureStrings(PyTypeObject *, const char *[]);
LIBSHIBOKEN_API int InitSignatureBytes(PyTypeObject *, const uint8_t[], size_t);
LIBSHIBOKEN_API int FinishSignatureInitialization(PyObject *, const char *[]);
LIBSHIBOKEN_API int FinishSignatureInitBytes(PyObject *, const uint8_t [], size_t);
LIBSHIBOKEN_API void SetError_Argument(PyObject *, const char *, PyObject *);
LIBSHIBOKEN_API PyObject *Sbk_TypeGet___doc__(PyObject *);
LIBSHIBOKEN_API PyObject *GetFeatureDict();

} // extern "C"

#endif // SIGNATURE_H
