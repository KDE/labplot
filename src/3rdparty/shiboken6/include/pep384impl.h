// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PEP384IMPL_H
#define PEP384IMPL_H

#include "shibokenmacros.h"

extern "C"
{

/*****************************************************************************
 *
 * RESOLVED: memoryobject.h
 *
 */

// Extracted into bufferprocs27.h
#ifdef Py_LIMITED_API
#include "bufferprocs_py37.h"
#endif

/*****************************************************************************
 *
 * RESOLVED: object.h
 *
 */
#ifdef Py_LIMITED_API
// Why the hell is this useful debugging function not allowed?
// BTW: When used, it breaks on Windows, intentionally!
LIBSHIBOKEN_API void _PyObject_Dump(PyObject *);
#endif

/*
 * There are a few structures that are needed, but cannot be used without
 * breaking the API. We use some heuristics to get those fields anyway
 * and validate that we really found them, see pep384impl.cpp .
 */

#ifdef Py_LIMITED_API

/*
 * These are the type object fields that we use.
 * We will verify that they never change.
 * The unused fields are intentionally named as "void *Xnn" because
 * the chance is smaller to forget to validate a field.
 * When we need more fields, we replace it back and add it to the
 * validation.
 */
typedef struct _typeobject {
    PyVarObject ob_base;
    const char *tp_name;
    Py_ssize_t tp_basicsize;
    void *X03; // Py_ssize_t tp_itemsize;
#ifdef PEP384_INTERN
    destructor tp_dealloc;
#else
    destructor X04;
#endif
    void *X05; // Py_ssize_t tp_vectorcall_offset;
    void *X06; // getattrfunc tp_getattr;
    void *X07; // setattrfunc tp_setattr;
    void *X08; // PyAsyncMethods *tp_as_async;
#ifdef PEP384_INTERN
    reprfunc tp_repr;
#else
    reprfunc X09;
#endif
    void *X10; // PyNumberMethods *tp_as_number;
    void *X11; // PySequenceMethods *tp_as_sequence;
    void *X12; // PyMappingMethods *tp_as_mapping;
    void *X13; // hashfunc tp_hash;
#ifdef PEP384_INTERN
    ternaryfunc tp_call;
#else
    ternaryfunc X14;
#endif
    reprfunc tp_str; // Only used for PEP384_INTERN and a shiboken test
    getattrofunc tp_getattro;
    setattrofunc tp_setattro;
    void *X18; // PyBufferProcs *tp_as_buffer;
    unsigned long tp_flags;
    void *X20; // const char *tp_doc;
#ifdef PEP384_INTERN
    traverseproc tp_traverse;
    inquiry tp_clear;
#else
    traverseproc X21;
    inquiry X22;
#endif
    void *X23; // richcmpfunc tp_richcompare;
    Py_ssize_t tp_weaklistoffset;
    void *X25; // getiterfunc tp_iter;
#ifdef PEP384_INTERN
    iternextfunc tp_iternext;
#else
    iternextfunc X26;
#endif
    struct PyMethodDef *tp_methods;
    struct PyMemberDef *tp_members;
    struct PyGetSetDef *tp_getset;
    struct _typeobject *tp_base;
#ifdef PEP384_INTERN
    PyObject *tp_dict;
    descrgetfunc tp_descr_get;
    descrsetfunc tp_descr_set;
#else
    void *X31;
    descrgetfunc X32;
    descrsetfunc X33;
#endif
    Py_ssize_t tp_dictoffset;
#ifdef PEP384_INTERN
    initproc tp_init;
    allocfunc tp_alloc;
#else
    initproc X39;
    allocfunc X40;
#endif
    newfunc tp_new;
#ifdef PEP384_INTERN
    freefunc tp_free;
    inquiry tp_is_gc; /* For PyObject_IS_GC */
#else
    freefunc X41;
    inquiry X42; /* For PyObject_IS_GC */
#endif
    PyObject *tp_bases;
    PyObject *tp_mro; /* method resolution order */

} PyTypeObject;

#ifndef PyObject_IS_GC
/* Test if an object has a GC head */
#define PyObject_IS_GC(o) \
    (PyType_IS_GC(Py_TYPE(o)) \
     && (Py_TYPE(o)->tp_is_gc == NULL || Py_TYPE(o)->tp_is_gc(o)))
#endif

LIBSHIBOKEN_API PyObject *_PepType_Lookup(PyTypeObject *type, PyObject *name);

#else // Py_LIMITED_API

#define _PepType_Lookup(type, name)       _PyType_Lookup(type, name)

#endif // Py_LIMITED_API

/// PYSIDE-939: We need the runtime version, given major << 16 + minor << 8 + micro
LIBSHIBOKEN_API long _PepRuntimeVersion();
/*****************************************************************************
 *
 * PYSIDE-535: Implement a clean type extension for PyPy
 *
 */

struct SbkObjectTypePrivate;

LIBSHIBOKEN_API SbkObjectTypePrivate *PepType_SOTP(PyTypeObject *type);
LIBSHIBOKEN_API void PepType_SOTP_delete(PyTypeObject *type);

struct SbkEnumType;
struct SbkEnumTypePrivate;

LIBSHIBOKEN_API SbkEnumTypePrivate *PepType_SETP(SbkEnumType *type);
LIBSHIBOKEN_API void PepType_SETP_delete(SbkEnumType *enumType);

struct PySideQFlagsType;
struct SbkQFlagsTypePrivate;

/*****************************************************************************/

// functions used everywhere
LIBSHIBOKEN_API const char *PepType_GetNameStr(PyTypeObject *type);

LIBSHIBOKEN_API PyObject *Pep_GetPartialFunction(void);

/*****************************************************************************
 *
 * RESOLVED: pydebug.h
 *
 */
#ifdef Py_LIMITED_API
/*
 * We have no direct access to Py_VerboseFlag because debugging is not
 * supported. The python developers are partially a bit too rigorous.
 * Instead, we compute the value and use a function call macro.
 * Was before: extern LIBSHIBOKEN_API int Py_VerboseFlag;
 */
LIBSHIBOKEN_API int Pep_GetFlag(const char *name);
LIBSHIBOKEN_API int Pep_GetVerboseFlag(void);
#endif

// pyerrors.h
#if defined(Py_LIMITED_API) || PY_VERSION_HEX < 0x030C0000
LIBSHIBOKEN_API PyObject *PepErr_GetRaisedException();
LIBSHIBOKEN_API PyObject *PepException_GetArgs(PyObject *ex);
LIBSHIBOKEN_API void PepException_SetArgs(PyObject *ex, PyObject *args);
#else
#  define PepErr_GetRaisedException PyErr_GetRaisedException
#  define PepException_GetArgs PyException_GetArgs
#  define PepException_SetArgs PyException_SetArgs
#endif

/*****************************************************************************
 *
 * RESOLVED: unicodeobject.h
 *
 */

///////////////////////////////////////////////////////////////////////
//
// PYSIDE-813: About The Length Of Unicode Objects
// -----------------------------------------------
//
// In Python 2 and before Python 3.3, the macro PyUnicode_GET_SIZE
// worked fine and really like a macro.
//
// Meanwhile, the unicode objects have changed their layout very much,
// and the former cheap macro call has become a real function call
// that converts objects and needs PyMemory.
//
// That is not only inefficient, but also requires the GIL!
// This problem was visible by debug Python and qdatastream_test.py .
// It was found while fixing the refcount problem of PYSIDE-813 which
// needed a debug Python.
//

// Unfortunately, we cannot ask this at runtime
// #if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x030A0000
// FIXME: Python 3.10: Replace _PepUnicode_AsString by PyUnicode_AsUTF8
#ifdef Py_LIMITED_API

LIBSHIBOKEN_API const char *_PepUnicode_AsString(PyObject *);

enum PepUnicode_Kind {
#if PY_VERSION_HEX < 0x030C0000
    PepUnicode_WCHAR_KIND = 0,
#endif
    PepUnicode_1BYTE_KIND = 1,
    PepUnicode_2BYTE_KIND = 2,
    PepUnicode_4BYTE_KIND = 4
};

LIBSHIBOKEN_API int _PepUnicode_KIND(PyObject *);
LIBSHIBOKEN_API int _PepUnicode_IS_ASCII(PyObject *str);
LIBSHIBOKEN_API int _PepUnicode_IS_COMPACT(PyObject *str);

LIBSHIBOKEN_API void *_PepUnicode_DATA(PyObject *str);

#else

enum PepUnicode_Kind {
#if PY_VERSION_HEX < 0x030C0000
    PepUnicode_WCHAR_KIND = PyUnicode_WCHAR_KIND,
#endif
    PepUnicode_1BYTE_KIND = PyUnicode_1BYTE_KIND,
    PepUnicode_2BYTE_KIND = PyUnicode_2BYTE_KIND,
    PepUnicode_4BYTE_KIND =  PyUnicode_4BYTE_KIND
};

#define _PepUnicode_AsString     PyUnicode_AsUTF8
#define _PepUnicode_KIND         PyUnicode_KIND
#define _PepUnicode_DATA         PyUnicode_DATA
#define _PepUnicode_IS_COMPACT   PyUnicode_IS_COMPACT
#define _PepUnicode_IS_ASCII     PyUnicode_IS_ASCII
#endif

/*****************************************************************************
 *
 * RESOLVED: methodobject.h
 *
 */

#ifdef Py_LIMITED_API

using PyCFunctionObject = struct _pycfunc;
#define PepCFunction_GET_NAMESTR(func) \
    _PepUnicode_AsString(PyObject_GetAttrString((PyObject *)func, "__name__"))
#else
#define PepCFunction_GET_NAMESTR(func) \
    (reinterpret_cast<const PyCFunctionObject *>(func)->m_ml->ml_name)
#endif

/*****************************************************************************
 *
 * RESOLVED: pythonrun.h
 *
 */
#ifdef Py_LIMITED_API
LIBSHIBOKEN_API PyObject *PyRun_String(const char *, int, PyObject *, PyObject *);
#endif

/*****************************************************************************
 *
 * RESOLVED: abstract.h
 *
 */
#ifdef Py_LIMITED_API

// This definition breaks the limited API a little, because it re-enables the
// buffer functions.
// But this is no problem as we check it's validity for every version.

// PYSIDE-1960 The buffer interface is since Python 3.11 part of the stable
// API and we do not need to check the compatibility by hand anymore.

typedef struct {
     getbufferproc bf_getbuffer;
     releasebufferproc bf_releasebuffer;
} PyBufferProcs;

typedef struct _Pepbuffertype {
    PyVarObject ob_base;
    void *skip[17];
    PyBufferProcs *tp_as_buffer;
} PepBufferType;

#define PepType_AS_BUFFER(type)   \
    reinterpret_cast<PepBufferType *>(type)->tp_as_buffer

#define PyObject_CheckBuffer(obj) \
    ((PepType_AS_BUFFER(Py_TYPE(obj)) != NULL) &&  \
     (PepType_AS_BUFFER(Py_TYPE(obj))->bf_getbuffer != NULL))

LIBSHIBOKEN_API int PyObject_GetBuffer(PyObject *ob, Pep_buffer *view, int flags);
LIBSHIBOKEN_API void PyBuffer_Release(Pep_buffer *view);

#else

#define Pep_buffer                          Py_buffer
#define PepType_AS_BUFFER(type)             ((type)->tp_as_buffer)

#endif /* Py_LIMITED_API */

/*****************************************************************************
 *
 * RESOLVED: funcobject.h
 *
 */
#ifdef Py_LIMITED_API
typedef struct _func PyFunctionObject;

extern LIBSHIBOKEN_API PyTypeObject *PepFunction_TypePtr;
LIBSHIBOKEN_API PyObject *PepFunction_Get(PyObject *, const char *);

#define PyFunction_Check(op)        (Py_TYPE(op) == PepFunction_TypePtr)
#define PyFunction_GET_CODE(func)   PyFunction_GetCode(func)

#define PyFunction_GetCode(func)    PepFunction_Get((PyObject *)func, "__code__")
#define PepFunction_GetName(func)   PepFunction_Get((PyObject *)func, "__name__")
#else
#define PepFunction_TypePtr         (&PyFunction_Type)
#define PepFunction_GetName(func)   (((PyFunctionObject *)func)->func_name)
#endif

/*****************************************************************************
 *
 * RESOLVED: classobject.h
 *
 */
#ifdef Py_LIMITED_API

typedef struct _meth PyMethodObject;

extern LIBSHIBOKEN_API PyTypeObject *PepMethod_TypePtr;

LIBSHIBOKEN_API PyObject *PyMethod_New(PyObject *, PyObject *);
LIBSHIBOKEN_API PyObject *PyMethod_Function(PyObject *);
LIBSHIBOKEN_API PyObject *PyMethod_Self(PyObject *);

#define PyMethod_Check(op) ((op)->ob_type == PepMethod_TypePtr)

#define PyMethod_GET_SELF(op)       PyMethod_Self(op)
#define PyMethod_GET_FUNCTION(op)   PyMethod_Function(op)
#endif

/*****************************************************************************
 *
 * RESOLVED: code.h
 *
 */
#ifdef Py_LIMITED_API
/* Bytecode object */

// we have to grab the code object from python
typedef struct _code PepCodeObject;

LIBSHIBOKEN_API int PepCode_Get(PepCodeObject *co, const char *name);
LIBSHIBOKEN_API int PepCode_Check(PyObject *o);

#  define PepCode_GET_FLAGS(o)         PepCode_Get(o, "co_flags")
#  define PepCode_GET_ARGCOUNT(o)      PepCode_Get(o, "co_argcount")

LIBSHIBOKEN_API PyObject *PepFunction_GetDefaults(PyObject *function);

/* Masks for co_flags above */
#  define CO_OPTIMIZED    0x0001
#  define CO_NEWLOCALS    0x0002
#  define CO_VARARGS      0x0004
#  define CO_VARKEYWORDS  0x0008
#  define CO_NESTED       0x0010
#  define CO_GENERATOR    0x0020

#else

#  define PepCodeObject                PyCodeObject
#  define PepCode_GET_FLAGS(o)         ((o)->co_flags)
#  define PepCode_GET_ARGCOUNT(o)      ((o)->co_argcount)
#  define PepCode_Check PyCode_Check

#  ifdef PYPY_VERSION

LIBSHIBOKEN_API PyObject *PepFunction_GetDefaults(PyObject *function);

#    else
#    define PepFunction_GetDefaults PyFunction_GetDefaults
#  endif
#endif

/*****************************************************************************
 *
 * RESOLVED: datetime.h
 *
 */
#ifdef Py_LIMITED_API

LIBSHIBOKEN_API int PyDateTime_Get(PyObject *ob, const char *name);

#define PyDateTime_GetYear(o)         PyDateTime_Get(o, "year")
#define PyDateTime_GetMonth(o)        PyDateTime_Get(o, "month")
#define PyDateTime_GetDay(o)          PyDateTime_Get(o, "day")
#define PyDateTime_GetHour(o)         PyDateTime_Get(o, "hour")
#define PyDateTime_GetMinute(o)       PyDateTime_Get(o, "minute")
#define PyDateTime_GetSecond(o)       PyDateTime_Get(o, "second")
#define PyDateTime_GetMicrosecond(o)  PyDateTime_Get(o, "microsecond")
#define PyDateTime_GetFold(o)         PyDateTime_Get(o, "fold")

#define PyDateTime_GET_YEAR(o)              PyDateTime_GetYear(o)
#define PyDateTime_GET_MONTH(o)             PyDateTime_GetMonth(o)
#define PyDateTime_GET_DAY(o)               PyDateTime_GetDay(o)

#define PyDateTime_DATE_GET_HOUR(o)         PyDateTime_GetHour(o)
#define PyDateTime_DATE_GET_MINUTE(o)       PyDateTime_GetMinute(o)
#define PyDateTime_DATE_GET_SECOND(o)       PyDateTime_GetSecond(o)
#define PyDateTime_DATE_GET_MICROSECOND(o)  PyDateTime_GetMicrosecond(o)
#define PyDateTime_DATE_GET_FOLD(o)         PyDateTime_GetFold(o)

#define PyDateTime_TIME_GET_HOUR(o)         PyDateTime_GetHour(o)
#define PyDateTime_TIME_GET_MINUTE(o)       PyDateTime_GetMinute(o)
#define PyDateTime_TIME_GET_SECOND(o)       PyDateTime_GetSecond(o)
#define PyDateTime_TIME_GET_MICROSECOND(o)  PyDateTime_GetMicrosecond(o)
#define PyDateTime_TIME_GET_FOLD(o)         PyDateTime_GetFold(o)

/* Define structure slightly similar to C API. */
typedef struct {
    PyObject *module;
    /* type objects */
    PyTypeObject *DateType;
    PyTypeObject *DateTimeType;
    PyTypeObject *TimeType;
    PyTypeObject *DeltaType;
    PyTypeObject *TZInfoType;
} datetime_struc;

LIBSHIBOKEN_API datetime_struc *init_DateTime(void);

#define PyDateTime_IMPORT     PyDateTimeAPI = init_DateTime()

extern LIBSHIBOKEN_API datetime_struc *PyDateTimeAPI;

#define PyDate_Check(op)      PyObject_TypeCheck(op, PyDateTimeAPI->DateType)
#define PyDateTime_Check(op)  PyObject_TypeCheck(op, PyDateTimeAPI->DateTimeType)
#define PyTime_Check(op)      PyObject_TypeCheck(op, PyDateTimeAPI->TimeType)

LIBSHIBOKEN_API PyObject *PyDate_FromDate(int year, int month, int day);
LIBSHIBOKEN_API PyObject *PyDateTime_FromDateAndTime(
    int year, int month, int day, int hour, int min, int sec, int usec);
LIBSHIBOKEN_API PyObject *PyTime_FromTime(
    int hour, int minute, int second, int usecond);

#endif /* Py_LIMITED_API */

/*****************************************************************************
 *
 * Extra support for name mangling
 *
 */

// PYSIDE-772: This function supports the fix, but is not meant as public.
LIBSHIBOKEN_API PyObject *_Pep_PrivateMangle(PyObject *self, PyObject *name);

/*****************************************************************************
 *
 * Extra support for signature.cpp
 *
 */

#ifdef Py_LIMITED_API
extern LIBSHIBOKEN_API PyTypeObject *PepStaticMethod_TypePtr;
LIBSHIBOKEN_API PyObject *PyStaticMethod_New(PyObject *callable);
#else
#define PepStaticMethod_TypePtr &PyStaticMethod_Type
#endif

#ifdef PYPY_VERSION
extern LIBSHIBOKEN_API PyTypeObject *PepBuiltinMethod_TypePtr;
#endif

// Although not PEP specific, we resolve this similar issue, here:
#define PepMethodDescr_TypePtr &PyMethodDescr_Type

/*****************************************************************************
 *
 * Newly introduced convenience functions
 *
 * This is not defined if Py_LIMITED_API is defined.
 */

// Evaluate a script and return the variable `result`
LIBSHIBOKEN_API PyObject *PepRun_GetResult(const char *command);

// Call PyType_Type.tp_new returning a PyType object.
LIBSHIBOKEN_API PyTypeObject *PepType_Type_tp_new(PyTypeObject *metatype,
                                                  PyObject *args,
                                                  PyObject *kwds);

/*****************************************************************************
 *
 * Runtime support for Python 3.8 incompatibilities
 *
 */

#ifndef Py_TPFLAGS_METHOD_DESCRIPTOR
/* Objects behave like an unbound method */
#define Py_TPFLAGS_METHOD_DESCRIPTOR (1UL << 17)
#endif

/*****************************************************************************
 *
 * Runtime support for Python 3.12 incompatibility
 *
 */

LIBSHIBOKEN_API PyObject *PepType_GetDict(PyTypeObject *type);

// This function does not exist as PyType_SetDict. But because tp_dict
// is no longer considered to be accessible, we treat it as such.
LIBSHIBOKEN_API int PepType_SetDict(PyTypeObject *type, PyObject *dict);

LIBSHIBOKEN_API void *PepType_GetSlot(PyTypeObject *type, int aSlot);

/*****************************************************************************
 *
 * Module Initialization
 *
 */

LIBSHIBOKEN_API void Pep384_Init(void);

} // extern "C"

#endif // PEP384IMPL_H
