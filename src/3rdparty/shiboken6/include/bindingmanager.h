// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include "sbkpython.h"
#include "shibokenmacros.h"

#include <set>
#include <utility>

struct SbkObject;

namespace Shiboken
{

namespace Module {
struct TypeInitStruct;
}

struct DestructorEntry;

using ObjectVisitor = void (*)(SbkObject *, void *);

class LIBSHIBOKEN_API BindingManager
{
public:
    BindingManager(const BindingManager &) = delete;
    BindingManager(BindingManager &&) = delete;
    BindingManager &operator=(const BindingManager &) = delete;
    BindingManager &operator=(BindingManager &&) = delete;

    static BindingManager &instance();

    bool hasWrapper(const void *cptr);

    void registerWrapper(SbkObject *pyObj, void *cptr);
    void releaseWrapper(SbkObject *wrapper);

    void runDeletionInMainThread();
    void addToDeletionInMainThread(const DestructorEntry &);

    SbkObject *retrieveWrapper(const void *cptr);
    PyObject *getOverride(const void *cptr, PyObject *nameCache[], const char *methodName);

    void addClassInheritance(Module::TypeInitStruct *parent, Module::TypeInitStruct *child);
    /// Try to find the correct type of cptr via type discovery knowing that it's at least
    /// of type \p type. If a derived class is found, it returns a cptr cast to the type
    /// (which may be different in case of  multiple inheritance.
    /// \param cptr a pointer to the instance of type \p type
    /// \param type type of cptr
    using TypeCptrPair = std::pair<PyTypeObject *, void *>;
    TypeCptrPair findDerivedType(void *cptr, PyTypeObject *type) const;

    /**
     * Try to find the correct type of *cptr knowing that it's at least of type \p type.
     * In case of multiple inheritance this function may change the contents of cptr.
     * \param cptr a pointer to a pointer to the instance of type \p type
     * \param type type of *cptr
     * \warning This function is slow, use it only as last resort.
     */
    [[deprecated]] PyTypeObject *resolveType(void **cptr, PyTypeObject *type);

    std::set<PyObject *> getAllPyObjects();

    /**
     * Calls the function \p visitor for each object registered on binding manager.
     * \note As various C++ pointers can point to the same PyObject due to multiple inheritance
     *       a PyObject can be called more than one time for each PyObject.
     * \param visitor function called for each object.
     * \param data user data passed as second argument to the visitor function.
     */
    void visitAllPyObjects(ObjectVisitor visitor, void *data);

    bool dumpTypeGraph(const char *fileName) const;
    void dumpWrapperMap();

private:
    ~BindingManager();
    BindingManager();

    struct BindingManagerPrivate;
    BindingManagerPrivate *m_d;
};

LIBSHIBOKEN_API bool callInheritedInit(PyObject *self, PyObject *args, PyObject *kwds,
                                       const char *fullName);
LIBSHIBOKEN_API bool callInheritedInit(PyObject *self, PyObject *args, PyObject *kwds,
                                       Module::TypeInitStruct typeStruct);

} // namespace Shiboken

#endif // BINDINGMANAGER_H
