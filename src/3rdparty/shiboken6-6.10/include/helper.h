// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef HELPER_H
#define HELPER_H

#include "sbkpython.h"
#include "shibokenmacros.h"
#include "autodecref.h"

#include <iosfwd>

#define SBK_UNUSED(x)   (void)(x);

namespace Shiboken
{

/**
* It transforms a python sequence into two C variables, argc and argv.
* This function tries to find the application (script) name and put it into argv[0], if
* the application name can't be guessed, defaultAppName will be used.
*
* No memory is allocated is an error occur.
*
* \note argc must be a valid address.
* \note The argv array is allocated using new operator and each item is allocated using malloc.
* \returns True on sucess, false otherwise.
*/
LIBSHIBOKEN_API bool listToArgcArgv(PyObject *argList, int *argc, char ***argv, const char *defaultAppName = nullptr);

/// Delete a a list of arguments created by listToArgcArgv()
LIBSHIBOKEN_API void deleteArgv(int argc, char **argv);

/**
 * Convert a python sequence into a heap-allocated array of ints.
 *
 * \returns The newly allocated array or NULL in case of error or empty sequence. Check with PyErr_Occurred
 *          if it was successfull.
 */
LIBSHIBOKEN_API int *sequenceToIntArray(PyObject *obj, bool zeroTerminated = false);

/// Fix a type name returned by typeid(t).name(), depending on compiler.
/// \returns Fixed name (allocated).
LIBSHIBOKEN_API const char *typeNameOf(const char *typeIdName);

/// Returns whether \a method is a compiled method (Nuitka).
LIBSHIBOKEN_API bool isCompiledMethod(PyObject *method);

/**
 *  Creates and automatically deallocates C++ arrays.
 */
template<class T>
class ArrayPointer
{
    public:
        ArrayPointer(const ArrayPointer &) = delete;
        ArrayPointer(ArrayPointer &&) = delete;
        ArrayPointer &operator=(const ArrayPointer &) = delete;
        ArrayPointer &operator=(ArrayPointer &&) = delete;

        explicit ArrayPointer(Py_ssize_t size) : data(new T[size]) {}
        T &operator[](Py_ssize_t pos) { return data[pos]; }
        operator T *() const { return data; }
        ~ArrayPointer() { delete[] data; }
    private:
        T *data;
};

template <class T>
using AutoArrayPointer = ArrayPointer<T>; // deprecated

using ThreadId = unsigned long long;
LIBSHIBOKEN_API ThreadId currentThreadId();
LIBSHIBOKEN_API ThreadId mainThreadId();

LIBSHIBOKEN_API int pyVerbose();

/**
 * An utility function used to call PyErr_WarnEx with a formatted message.
 */
LIBSHIBOKEN_API int warning(PyObject *category, int stacklevel, const char *format, ...);

struct LIBSHIBOKEN_API debugPyObject
{
    explicit debugPyObject(PyObject *o);

    PyObject *m_object;
};

struct LIBSHIBOKEN_API debugSbkObject
{
    explicit debugSbkObject(SbkObject *o);

    SbkObject *m_object;
};

struct LIBSHIBOKEN_API debugPyTypeObject
{
    explicit debugPyTypeObject(PyTypeObject *o);

    PyTypeObject *m_object;
};

struct debugPyBuffer;

struct debugPyArrayObject
{
    explicit debugPyArrayObject(PyObject *object) : m_object(object) {}

    PyObject *m_object;
};

LIBSHIBOKEN_API std::ostream &operator<<(std::ostream &str, const debugPyObject &o);
LIBSHIBOKEN_API std::ostream &operator<<(std::ostream &str, const debugSbkObject &o);
LIBSHIBOKEN_API std::ostream &operator<<(std::ostream &str, const debugPyTypeObject &o);
LIBSHIBOKEN_API std::ostream &operator<<(std::ostream &str, const debugPyBuffer &b);
LIBSHIBOKEN_API std::ostream &operator<<(std::ostream &str, const debugPyArrayObject &b);
LIBSHIBOKEN_API std::ios_base &debugVerbose(std::ios_base &s);
LIBSHIBOKEN_API std::ios_base &debugBrief(std::ios_base &s);
} // namespace Shiboken


#endif // HELPER_H
