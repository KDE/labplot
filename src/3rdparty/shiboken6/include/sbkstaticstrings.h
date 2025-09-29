// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SBKSTATICSTRINGS_H
#define SBKSTATICSTRINGS_H

#include "sbkpython.h"
#include "shibokenmacros.h"

namespace Shiboken
{
// Some often-used strings
namespace PyName
{
LIBSHIBOKEN_API PyObject *co_name();
LIBSHIBOKEN_API PyObject *dumps();
LIBSHIBOKEN_API PyObject *fget();
LIBSHIBOKEN_API PyObject *fset();
LIBSHIBOKEN_API PyObject *f_code();
LIBSHIBOKEN_API PyObject *f_lineno();
LIBSHIBOKEN_API PyObject *im_func();
LIBSHIBOKEN_API PyObject *im_self();
LIBSHIBOKEN_API PyObject *loads();
LIBSHIBOKEN_API PyObject *multi();
LIBSHIBOKEN_API PyObject *name();
LIBSHIBOKEN_API PyObject *orig_dict();
LIBSHIBOKEN_API PyObject *result();
LIBSHIBOKEN_API PyObject *select_id();
LIBSHIBOKEN_API PyObject *value();
LIBSHIBOKEN_API PyObject *values();
LIBSHIBOKEN_API PyObject *qtStaticMetaObject();
} // namespace PyName

namespace PyMagicName
{
LIBSHIBOKEN_API PyObject *class_();
LIBSHIBOKEN_API PyObject *dict();
LIBSHIBOKEN_API PyObject *doc();
LIBSHIBOKEN_API PyObject *ecf();
LIBSHIBOKEN_API PyObject *file();
LIBSHIBOKEN_API PyObject *func();
LIBSHIBOKEN_API PyObject *get();
LIBSHIBOKEN_API PyObject *members();
LIBSHIBOKEN_API PyObject *module();
LIBSHIBOKEN_API PyObject *name();
LIBSHIBOKEN_API PyObject *property_methods();
LIBSHIBOKEN_API PyObject *qualname();
LIBSHIBOKEN_API PyObject *self();
LIBSHIBOKEN_API PyObject *opaque_container();
LIBSHIBOKEN_API PyObject *code();
LIBSHIBOKEN_API PyObject *rlshift();
LIBSHIBOKEN_API PyObject *rrshift();
} // namespace PyMagicName

namespace Messages
{
LIBSHIBOKEN_API PyObject *unknownException();
} // Messages
} // namespace Shiboken

#endif // SBKSTATICSTRINGS_H
