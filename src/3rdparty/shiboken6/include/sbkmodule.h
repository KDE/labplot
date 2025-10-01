// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBK_MODULE_H
#define SBK_MODULE_H

#include "sbkpython.h"
#include "shibokenmacros.h"

extern "C"
{
struct SbkConverter;
}

namespace Shiboken::Module {

struct TypeInitStruct
{
    PyTypeObject *type;
    const char *fullName;
};

/// PYSIDE-2404: Replacing the arguments in cpythonTypeNameExt by a function.
LIBSHIBOKEN_API PyTypeObject *get(TypeInitStruct &typeStruct);

/// PYSIDE-2404: Make sure that mentioned classes really exist.
LIBSHIBOKEN_API void loadLazyClassesWithName(const char *name);

/// PYSIDE-2404: incarnate all classes for star imports.
LIBSHIBOKEN_API void resolveLazyClasses(PyObject *module);

/**
 *  Imports and returns the module named \p moduleName, or a NULL pointer in case of failure.
 *  If the module is already imported, it increments its reference count before returning it.
 *  \returns the module specified in \p moduleName or NULL if an error occurs.
 */
LIBSHIBOKEN_API PyObject *import(const char *moduleName);

/**
 *  Creates a new Python module named \p moduleName using the information passed in \p moduleData.
 *  In fact, \p moduleData expects a "PyMethodDef *" object, but that's for Python 2. A "void*"
 *  was preferred to make this work with future Python 3 support.
 *  \returns a newly created module.
 */
LIBSHIBOKEN_API PyObject *create(const char *moduleName, void *moduleData);

using TypeCreationFunction = PyTypeObject *(*)(PyObject *module);

/// Adds a type creation function to the module.
LIBSHIBOKEN_API void AddTypeCreationFunction(PyObject *module,
                                             const char *name,
                                             TypeCreationFunction func);

LIBSHIBOKEN_API void AddTypeCreationFunction(PyObject *module,
                                             const char *name,
                                             TypeCreationFunction func,
                                             const char *containerName);

/**
 *  Registers the list of types created by \p module.
 *  \param module   Module where the types were created.
 *  \param types    Array of PyTypeObject *objects representing the types created on \p module.
 */
LIBSHIBOKEN_API void registerTypes(PyObject *module, TypeInitStruct *types);

/**
 *  Retrieves the array of types.
 *  \param module   Module where the types were created.
 *  \returns        A pointer to the PyTypeObject *array of types.
 */
LIBSHIBOKEN_API TypeInitStruct *getTypes(PyObject *module);

/**
 *  Registers the list of converters created by \p module for non-wrapper types.
 *  \param module       Module where the converters were created.
 *  \param converters   Array of SbkConverter *objects representing the converters created on \p module.
 */
LIBSHIBOKEN_API void registerTypeConverters(PyObject *module, SbkConverter **converters);

/**
 *  Retrieves the array of converters.
 *  \param module   Module where the converters were created.
 *  \returns        A pointer to the SbkConverter *array of converters.
 */
LIBSHIBOKEN_API SbkConverter **getTypeConverters(PyObject *module);

/**
 *  Replace the dictionary of a module. This allows to use `__missing__`.
 */
LIBSHIBOKEN_API bool replaceModuleDict(PyObject *module, PyObject *modClass, PyObject *dict);

} // namespace Shiboken::Module

#endif // SBK_MODULE_H
