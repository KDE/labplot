// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTODECREF_H
#define AUTODECREF_H

#include "sbkpython.h"

#include <utility>

struct SbkObject;
namespace Shiboken
{

/**
 *  AutoDecRef holds a PyObject pointer and decrement its reference counter when destroyed.
 */
struct AutoDecRef
{
public:
    AutoDecRef(const AutoDecRef &) = delete;
    AutoDecRef(AutoDecRef &&o) noexcept : m_pyObj{std::exchange(o.m_pyObj, nullptr)} {}
    AutoDecRef &operator=(const AutoDecRef &) = delete;
    AutoDecRef &operator=(AutoDecRef &&o) noexcept
    {
        m_pyObj = std::exchange(o.m_pyObj, nullptr);
        return *this;
    }

    /// AutoDecRef constructor.
    /// \param pyobj A borrowed reference to a Python object
    explicit AutoDecRef(PyObject *pyObj) noexcept : m_pyObj(pyObj) {}
    /// AutoDecRef constructor.
    /// \param pyobj A borrowed reference to a wrapped Python object
    explicit AutoDecRef(SbkObject *pyObj) noexcept : m_pyObj(reinterpret_cast<PyObject *>(pyObj)) {}
    /// AutoDecref default constructor.
    /// To be used later with reset():
    AutoDecRef() noexcept = default;

    /// Decref the borrowed python reference
    ~AutoDecRef()
    {
        Py_XDECREF(m_pyObj);
    }

    [[nodiscard]] bool isNull() const { return m_pyObj == nullptr; }
    /// Returns the pointer of the Python object being held.
    [[nodiscard]] PyObject *object() const { return m_pyObj; }
    [[nodiscard]] operator PyObject *() const { return m_pyObj; }
    operator bool() const { return m_pyObj != nullptr; }
    PyObject *operator->() { return m_pyObj; }

    template<typename T>
    [[deprecated]] T cast()
    {
        return reinterpret_cast<T>(m_pyObj);
    }

    /**
     * Decref the current borrowed python reference and borrow \p other.
     */
    void reset(PyObject *other)
    {
        // Safely decref m_pyObj. See Py_XSETREF in object.h .
        PyObject *_py_tmp = m_pyObj;
        m_pyObj = other;
        Py_XDECREF(_py_tmp);
    }

    PyObject *release()
    {
        PyObject *result = m_pyObj;
        m_pyObj = nullptr;
        return result;
    }

private:
    PyObject *m_pyObj = nullptr;
};

} // namespace Shiboken

#endif // AUTODECREF_H
