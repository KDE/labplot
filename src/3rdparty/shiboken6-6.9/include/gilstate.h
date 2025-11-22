// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GILSTATE_H
#define GILSTATE_H

#include <shibokenmacros.h>
#include "sbkpython.h"

namespace Shiboken
{

class LIBSHIBOKEN_API GilState
{
public:
    GilState(const GilState &) = delete;
    GilState(GilState &&) = delete;
    GilState &operator=(const GilState &) = delete;
    GilState &operator=(GilState &&) = delete;

    explicit GilState(bool acquire=true);
    ~GilState();
    void acquire();
    void release();
    void abandon();
private:
    PyGILState_STATE m_gstate;
    bool m_locked = false;
};

} // namespace Shiboken

#endif // GILSTATE_H
