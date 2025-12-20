// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBK_BINDINGUTILS
#define SBK_BINDINGUTILS

#include "sbkpython.h"
#include "shibokenmacros.h"

namespace Shiboken {
struct AutoDecRef;

/// Maps a keyword argument by name to its parameter index
struct ArgumentNameIndexMapping
{
    const char *name;
    int index;
};

/// Function binding helper: Parse the keyword arguments in dict \a kwds
/// according to \a mapping (name->index) and store them in array \a pyArgs
/// under their index. Fails if an entry is missing or duplicate entries
/// occur.
LIBSHIBOKEN_API bool
    parseKeywordArguments(PyObject *kwds,
                          const ArgumentNameIndexMapping *mapping, size_t size,
                          Shiboken::AutoDecRef &errInfo, PyObject **pyArgs);

/// Function binding helper: Parse the keyword arguments of a QObject constructor
/// in dict \a kwds according to \a mapping (name->index) and store them in array
/// \a pyArgs under their index. Fails if duplicate entries occur. Unmapped entries
/// (QObject properties) are stored in a dict in errInfo for further processing.
LIBSHIBOKEN_API bool
    parseConstructorKeywordArguments(PyObject *kwds,
                                     const ArgumentNameIndexMapping *mapping, size_t size,
                                     Shiboken::AutoDecRef &errInfo, PyObject **pyArgs);

/// Returns whether we are running in compiled mode (Nuitka).
LIBSHIBOKEN_API bool isCompiled();

} // namespace Shiboken

#endif // SBK_BINDINGUTILS
