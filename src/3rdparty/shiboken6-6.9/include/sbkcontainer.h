// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBK_CONTAINER_H
#define SBK_CONTAINER_H

#include "sbkpython.h"
#include "shibokenmacros.h"
#include "shibokenbuffer.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <utility>

extern "C"
{
struct LIBSHIBOKEN_API ShibokenContainer
{
    PyObject_HEAD
    void *d;
};

} // extern "C"

// Conversion helper traits for container values (Keep it out of namespace as
// otherwise clashes occur).
template <class Value>
struct ShibokenContainerValueConverter
{
    static bool checkValue(PyObject *pyArg);
    static PyObject *convertValueToPython(Value v);
    static std::optional<Value> convertValueToCpp(PyObject pyArg);
};

// SFINAE test for the presence of reserve() in a sequence container (std::vector/QList)
template <typename T>
class ShibokenContainerHasReserve
{
private:
    using YesType = char[1];
    using NoType = char[2];

    template <typename C> static YesType& test( decltype(&C::reserve) ) ;
    template <typename C> static NoType& test(...);

public:
    enum { value = sizeof(test<T>(nullptr)) == sizeof(YesType) };
};

template <class SequenceContainer>
class ShibokenSequenceContainerPrivate // Helper for sequence type containers
{
public:
    using value_type = typename SequenceContainer::value_type;
    using OptionalValue = typename std::optional<value_type>;

    SequenceContainer *m_list{};
    bool m_ownsList = false;
    bool m_const = false;
    static constexpr const char *msgModifyConstContainer =
        "Attempt to modify a constant container.";

    static PyObject *tpNew(PyTypeObject *subtype, PyObject * /* args */, PyObject * /* kwds */)
    {
        allocfunc allocFunc = reinterpret_cast<allocfunc>(PepType_GetSlot(subtype, Py_tp_alloc));
        auto *me = reinterpret_cast<ShibokenContainer *>(allocFunc(subtype, 0));
        auto *d = new ShibokenSequenceContainerPrivate;
        d->m_list = new SequenceContainer;
        d->m_ownsList = true;
        me->d = d;
        return reinterpret_cast<PyObject *>(me);
    }

    static PyObject *tpNewInvalid(PyTypeObject * /* subtype */, PyObject * /* args */, PyObject * /* kwds */)
    {
        return PyErr_Format(PyExc_NotImplementedError,
                     "Opaque containers of type '%s' cannot be instantiated.",
                     typeid(SequenceContainer).name());
    }

    static int tpInit(PyObject * /* self */, PyObject * /* args */, PyObject * /* kwds */)
    {
        return 0;
    }

    static void tpFree(void *self)
    {
        auto *pySelf = reinterpret_cast<PyObject *>(self);
        auto *d = get(pySelf);
        if (d->m_ownsList)
            delete d->m_list;
        delete d;
        auto freeFunc = reinterpret_cast<freefunc>(PepType_GetSlot(Py_TYPE(pySelf)->tp_base,
                                                                   Py_tp_free));
        freeFunc(self);
    }

    static Py_ssize_t sqLen(PyObject *self)
    {
        return get(self)->m_list->size();
    }

    static PyObject *sqGetItem(PyObject *self, Py_ssize_t i)
    {
        auto *d = get(self);
        if (i < 0 || i >= Py_ssize_t(d->m_list->size()))
            return PyErr_Format(PyExc_IndexError, "index out of bounds");
        auto it = std::cbegin(*d->m_list);
        std::advance(it, i);
        return ShibokenContainerValueConverter<value_type>::convertValueToPython(*it);
    }

    static int sqSetItem(PyObject *self, Py_ssize_t i, PyObject *pyArg)
    {
        auto *d = get(self);
        if (i < 0 || i >= Py_ssize_t(d->m_list->size())) {
            PyErr_SetString(PyExc_IndexError, "index out of bounds");
            return -1;
        }
        auto it = std::begin(*d->m_list);
        std::advance(it, i);
        OptionalValue value = ShibokenContainerValueConverter<value_type>::convertValueToCpp(pyArg);
        if (!value.has_value())
            return -1;
        *it = value.value();
        return 0;
    }

    static PyObject *push_back(PyObject *self, PyObject *pyArg)
    {
        auto *d = get(self);
        if (!ShibokenContainerValueConverter<value_type>::checkValue(pyArg))
            return PyErr_Format(PyExc_TypeError, "wrong type passed to append.");
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        OptionalValue value = ShibokenContainerValueConverter<value_type>::convertValueToCpp(pyArg);
        if (!value.has_value())
            return nullptr;
        d->m_list->push_back(value.value());
        Py_RETURN_NONE;
    }

    static PyObject *push_front(PyObject *self, PyObject *pyArg)
    {
        auto *d = get(self);
        if (!ShibokenContainerValueConverter<value_type>::checkValue(pyArg))
            return PyErr_Format(PyExc_TypeError, "wrong type passed to append.");
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        OptionalValue value = ShibokenContainerValueConverter<value_type>::convertValueToCpp(pyArg);
        if (!value.has_value())
            return nullptr;
        d->m_list->push_front(value.value());
        Py_RETURN_NONE;
    }

    static PyObject *clear(PyObject *self)
    {
        auto *d = get(self);
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        d->m_list->clear();
        Py_RETURN_NONE;
    }

    static PyObject *pop_back(PyObject *self)
    {
        auto *d = get(self);
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        d->m_list->pop_back();
        Py_RETURN_NONE;
    }

    static PyObject *pop_front(PyObject *self)
    {
        auto *d = get(self);
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        d->m_list->pop_front();
        Py_RETURN_NONE;
    }

    // Support for containers with reserve/capacity
    static PyObject *reserve(PyObject *self, PyObject *pyArg)
    {
        auto *d = get(self);
        if (PyLong_Check(pyArg) == 0)
            return PyErr_Format(PyExc_TypeError, "wrong type passed to reserve().");
        if (d->m_const)
            return PyErr_Format(PyExc_TypeError, msgModifyConstContainer);

        if constexpr (ShibokenContainerHasReserve<SequenceContainer>::value) {
            const Py_ssize_t size = PyLong_AsSsize_t(pyArg);
            d->m_list->reserve(size);
        } else {
            return PyErr_Format(PyExc_TypeError, "Container does not support reserve().");
        }

        Py_RETURN_NONE;
    }

    static PyObject *capacity(PyObject *self)
    {
        Py_ssize_t result = -1;
        if constexpr (ShibokenContainerHasReserve<SequenceContainer>::value) {
            const auto *d = get(self);
            result = d->m_list->capacity();
        }
        return PyLong_FromSsize_t(result);
    }

    static PyObject *data(PyObject *self)
    {
        PyObject *result = nullptr;
        if constexpr (ShibokenContainerHasReserve<SequenceContainer>::value) {
            const auto *d = get(self);
            auto *data = d->m_list->data();
            const Py_ssize_t size = sizeof(value_type) * d->m_list->size();
            result = Shiboken::Buffer::newObject(data, size, Shiboken::Buffer::ReadWrite);
        } else  {
            PyErr_SetString(PyExc_TypeError, "Container does not support data().");
        }
        return result;
    }

    static PyObject *constData(PyObject *self)
    {
        PyObject *result = nullptr;
        if constexpr (ShibokenContainerHasReserve<SequenceContainer>::value) {
            const auto *d = get(self);
            const auto *data = std::as_const(d->m_list)->data();
            const Py_ssize_t size = sizeof(value_type) * d->m_list->size();
            result = Shiboken::Buffer::newObject(data, size);
        } else  {
            PyErr_SetString(PyExc_TypeError, "Container does not support constData().");
        }
        return result;
    }

    static ShibokenSequenceContainerPrivate *get(PyObject *self)
    {
        auto *data = reinterpret_cast<ShibokenContainer *>(self);
        return reinterpret_cast<ShibokenSequenceContainerPrivate *>(data->d);
    }
};

namespace Shiboken
{
LIBSHIBOKEN_API bool isOpaqueContainer(PyObject *o);
}

#endif // SBK_CONTAINER_H
