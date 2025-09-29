// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef THREADSTATESAVER_H
#define THREADSTATESAVER_H

#include "sbkpython.h"
#include <shibokenmacros.h>

namespace Shiboken
{

class LIBSHIBOKEN_API ThreadStateSaver
{
public:
    ThreadStateSaver(const ThreadStateSaver &) = delete;
    ThreadStateSaver(ThreadStateSaver &&) = delete;
    ThreadStateSaver &operator=(const ThreadStateSaver &) = delete;
    ThreadStateSaver &operator=(ThreadStateSaver &&) = delete;

    ThreadStateSaver();
    ~ThreadStateSaver();
    void save();
    void restore();
private:
    PyThreadState *m_threadState = nullptr;
};

} // namespace Shiboken

#endif // THREADSTATESAVER_H
