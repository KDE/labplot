// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
PSF LICENSE AGREEMENT FOR PYTHON 3.7.0

1. This LICENSE AGREEMENT is between the Python Software Foundation ("PSF"), and
   the Individual or Organization ("Licensee") accessing and otherwise using Python
   3.7.0 software in source or binary form and its associated documentation.

2. Subject to the terms and conditions of this License Agreement, PSF hereby
   grants Licensee a nonexclusive, royalty-free, world-wide license to reproduce,
   analyze, test, perform and/or display publicly, prepare derivative works,
   distribute, and otherwise use Python 3.7.0 alone or in any derivative
   version, provided, however, that PSF's License Agreement and PSF's notice of
   copyright, i.e., "Copyright © 2001-2018 Python Software Foundation; All Rights
   Reserved" are retained in Python 3.7.0 alone or in any derivative version
   prepared by Licensee.

3. In the event Licensee prepares a derivative work that is based on or
   incorporates Python 3.7.0 or any part thereof, and wants to make the
   derivative work available to others as provided herein, then Licensee hereby
   agrees to include in any such work a brief summary of the changes made to Python
   3.7.0.

4. PSF is making Python 3.7.0 available to Licensee on an "AS IS" basis.
   PSF MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  BY WAY OF
   EXAMPLE, BUT NOT LIMITATION, PSF MAKES NO AND DISCLAIMS ANY REPRESENTATION OR
   WARRANTY OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE
   USE OF PYTHON 3.7.0 WILL NOT INFRINGE ANY THIRD PARTY RIGHTS.

5. PSF SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF PYTHON 3.7.0
   FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS AS A RESULT OF
   MODIFYING, DISTRIBUTING, OR OTHERWISE USING PYTHON 3.7.0, OR ANY DERIVATIVE
   THEREOF, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

6. This License Agreement will automatically terminate upon a material breach of
   its terms and conditions.

7. Nothing in this License Agreement shall be deemed to create any relationship
   of agency, partnership, or joint venture between PSF and Licensee.  This License
   Agreement does not grant permission to use PSF trademarks or trade name in a
   trademark sense to endorse or promote products or services of Licensee, or any
   third party.

8. By copying, installing or otherwise using Python 3.7.0, Licensee agrees
   to be bound by the terms and conditions of this License Agreement.
*/

#ifndef BUFFER_REENABLE_H
#define BUFFER_REENABLE_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#ifdef Py_LIMITED_API

// The buffer interface has been added to limited API in 3.11, (abstract.h, PYSIDE-1960,
// except some internal structs).

#  if Py_LIMITED_API < 0x030B0000

struct Pep_buffer
{
    void *buf;
    PyObject *obj;        /* owned reference */
    Py_ssize_t len;
    Py_ssize_t itemsize;  /* This is Py_ssize_t so it can be
                             pointed to by strides in simple case.*/
    int readonly;
    int ndim;
    char *format;
    Py_ssize_t *shape;
    Py_ssize_t *strides;
    Py_ssize_t *suboffsets;
    void *internal;
};

using getbufferproc =int (*)(PyObject *, Pep_buffer *, int);
using releasebufferproc = void (*)(PyObject *, Pep_buffer *);
using Py_buffer = Pep_buffer;

#  else //  < 3.11

using Pep_buffer = Py_buffer;

#  endif // >= 3.11

// The structs below are not part of the limited API.
struct PepBufferProcs
{
    getbufferproc bf_getbuffer;
    releasebufferproc bf_releasebuffer;
};

struct PepBufferType
{
    PyVarObject ob_base;
    void *skip[17];
    PepBufferProcs *tp_as_buffer;
};

#  if Py_LIMITED_API < 0x030B0000

/* Maximum number of dimensions */
#define PyBUF_MAX_NDIM 64

/* Flags for getting buffers */
#define PyBUF_SIMPLE 0
#define PyBUF_WRITABLE 0x0001
/*  we used to include an E, backwards compatible alias  */
#define PyBUF_WRITEABLE PyBUF_WRITABLE
#define PyBUF_FORMAT 0x0004
#define PyBUF_ND 0x0008
#define PyBUF_STRIDES (0x0010 | PyBUF_ND)
#define PyBUF_C_CONTIGUOUS (0x0020 | PyBUF_STRIDES)
#define PyBUF_F_CONTIGUOUS (0x0040 | PyBUF_STRIDES)
#define PyBUF_ANY_CONTIGUOUS (0x0080 | PyBUF_STRIDES)
#define PyBUF_INDIRECT (0x0100 | PyBUF_STRIDES)

#define PyBUF_CONTIG (PyBUF_ND | PyBUF_WRITABLE)
#define PyBUF_CONTIG_RO (PyBUF_ND)

#define PyBUF_STRIDED (PyBUF_STRIDES | PyBUF_WRITABLE)
#define PyBUF_STRIDED_RO (PyBUF_STRIDES)

#define PyBUF_RECORDS (PyBUF_STRIDES | PyBUF_WRITABLE | PyBUF_FORMAT)
#define PyBUF_RECORDS_RO (PyBUF_STRIDES | PyBUF_FORMAT)

#define PyBUF_FULL (PyBUF_INDIRECT | PyBUF_WRITABLE | PyBUF_FORMAT)
#define PyBUF_FULL_RO (PyBUF_INDIRECT | PyBUF_FORMAT)


#define PyBUF_READ  0x100
#define PyBUF_WRITE 0x200

/* End buffer interface */
LIBSHIBOKEN_API PyObject *PyMemoryView_FromBuffer(Pep_buffer *info);
#define Py_buffer Pep_buffer

#define PyObject_CheckBuffer(obj) \
    ((PepType_AS_BUFFER(Py_TYPE(obj)) != NULL) &&  \
     (PepType_AS_BUFFER(Py_TYPE(obj))->bf_getbuffer != NULL))

LIBSHIBOKEN_API int PyObject_GetBuffer(PyObject *ob, Pep_buffer *view, int flags);
LIBSHIBOKEN_API void PyBuffer_Release(Pep_buffer *view);

#  endif // << 3.11

#  define PepType_AS_BUFFER(type)   \
reinterpret_cast<PepBufferType *>(type)->tp_as_buffer

using PyBufferProcs = PepBufferProcs;

#else // Py_LIMITED_API

using PepBufferProcs = PyBufferProcs;

#  define PepType_AS_BUFFER(type)             ((type)->tp_as_buffer)

#endif // !Py_LIMITED_API

#endif // BUFFER_REENABLE_H
