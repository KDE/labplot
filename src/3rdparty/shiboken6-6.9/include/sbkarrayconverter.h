// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKARRAYCONVERTERS_H
#define SBKARRAYCONVERTERS_H

#include "sbkpython.h"
#include "shibokenmacros.h"

extern "C" {
struct SbkArrayConverter;
}

namespace Shiboken::Conversions {

enum : int {
    SBK_UNIMPLEMENTED_ARRAY_IDX,
    SBK_DOUBLE_ARRAY_IDX,
    SBK_FLOAT_ARRAY_IDX,
    SBK_SHORT_ARRAY_IDX,
    SBK_UNSIGNEDSHORT_ARRAY_IDX,
    SBK_INT_ARRAY_IDX,
    SBK_UNSIGNEDINT_ARRAY_IDX,
    SBK_LONGLONG_ARRAY_IDX,
    SBK_UNSIGNEDLONGLONG_ARRAY_IDX,
    SBK_ARRAY_IDX_SIZE
};

/**
 * ArrayHandle is the type expected by shiboken6's array converter
 * functions. It provides access to array data which it may own
 * (in the case of conversions from PySequence) or a flat pointer
 * to internal data (in the case of array modules like numpy).
 */

template <class T>
class ArrayHandle
{
public:
    ArrayHandle(const ArrayHandle &) = delete;
    ArrayHandle& operator=(const ArrayHandle &) = delete;
    ArrayHandle(ArrayHandle &&) = delete;
    ArrayHandle& operator=(ArrayHandle &&) = delete;

    ArrayHandle() = default;
    ~ArrayHandle() { destroy(); }

    void allocate(Py_ssize_t size);
    void setData(T *d, size_t size);

    size_t size() const { return m_size; }
    T *data() const { return m_data; }
    operator T *() const { return m_data; }

private:
    void destroy();

    T *m_data = nullptr;
    Py_ssize_t m_size = 0;
    bool m_owned = false;
};

/**
 * Similar to ArrayHandle for fixed size 2 dimensional arrays.
 * columns is the size of the last dimension
 * It only has a setData() methods since it will be used for numpy only.
 */

template <class T, int columns>
class Array2Handle
{
public:
    using RowType = T[columns];

    Array2Handle() = default;

    operator RowType *() const { return m_rows; }

    void setData(RowType *d) { m_rows = d; }

private:
   RowType *m_rows = nullptr;
};

/// Returns the converter for an array type.
LIBSHIBOKEN_API SbkArrayConverter *arrayTypeConverter(int index, int dimension = 1);

template <class T>
struct ArrayTypeIndex{
    enum : int { index = SBK_UNIMPLEMENTED_ARRAY_IDX };
};

template <> struct ArrayTypeIndex<double> { enum : int { index = SBK_DOUBLE_ARRAY_IDX }; };
template <> struct ArrayTypeIndex<float> { enum : int { index = SBK_FLOAT_ARRAY_IDX };};
template <> struct ArrayTypeIndex<short> { enum : int { index = SBK_SHORT_ARRAY_IDX };};
template <> struct ArrayTypeIndex<unsigned short> { enum : int { index = SBK_UNSIGNEDSHORT_ARRAY_IDX };};
template <> struct ArrayTypeIndex<int> { enum : int { index = SBK_INT_ARRAY_IDX };};
template <> struct ArrayTypeIndex<unsigned> { enum : int { index = SBK_UNSIGNEDINT_ARRAY_IDX };};
template <> struct ArrayTypeIndex<long long> { enum : int { index = SBK_LONGLONG_ARRAY_IDX };};
template <> struct ArrayTypeIndex<unsigned long long> { enum : int { index = SBK_UNSIGNEDLONGLONG_ARRAY_IDX };};

template<typename T> SbkArrayConverter *ArrayTypeConverter(int dimension)
{ return arrayTypeConverter(ArrayTypeIndex<T>::index, dimension); }

// ArrayHandle methods
template<class T>
void ArrayHandle<T>::allocate(Py_ssize_t size)
{
    destroy();
    m_data = new T[size];
    m_size = size;
    m_owned = true;
}

template<class T>
void ArrayHandle<T>::setData(T *d, size_t size)
{
    destroy();
    m_data = d;
    m_size = size;
    m_owned = false;
}

template<class T>
void ArrayHandle<T>::destroy()
{
    if (m_owned)
        delete [] m_data;
    m_data = nullptr;
    m_size = 0;
    m_owned = false;
}

} // namespace Shiboken::Conversions

#endif // SBKARRAYCONVERTERS_H
