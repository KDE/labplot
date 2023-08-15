/*
    This file is part of the KD MacTouchBar library.
    SPDX-FileCopyrightText: 2019-2020 Klaralvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef KDMACTOUCHBAR_GLOBAL_H
#define KDMACTOUCHBAR_GLOBAL_H

#include <QtCore/QtGlobal>

#ifdef KDMACTOUCHBAR_BUILD_KDMACTOUCHBAR_LIB
# define KDMACTOUCHBAR_EXPORT Q_DECL_EXPORT
#else
# define KDMACTOUCHBAR_EXPORT Q_DECL_IMPORT
#endif

#endif /* KDMACTOUCHBAR_GLOBAL_H */
