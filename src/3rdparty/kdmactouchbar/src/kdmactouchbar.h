/*
    This file is part of the KD MacTouchBar library.
    SPDX-FileCopyrightText: 2019-2020 Klaralvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef KDMACTOUCHBAR_H
#define KDMACTOUCHBAR_H

#include "kdmactouchbar_global.h"

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QDialogButtonBox;
class QMessageBox;
class QTabBar;


class KDMACTOUCHBAR_EXPORT KDMacTouchBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QAction *principialAction READ principialAction WRITE setPrincipialAction)
    Q_PROPERTY(QAction *escapeAction READ escapeAction WRITE setEscapeAction)
    Q_PROPERTY(TouchButtonStyle touchButtonStyle READ touchButtonStyle WRITE setTouchButtonStyle)
public:
    explicit KDMacTouchBar(QWidget *parent = nullptr);
    explicit KDMacTouchBar(QMessageBox *messageBox);
    ~KDMacTouchBar();

    enum TouchButtonStyle
    {
        IconOnly,
        TextOnly,
        TextBesideIcon
    };

    static void setAutomaticallyCreateMessageBoxTouchBar(bool automatic);
    static bool isAutomacicallyCreatingMessageBoxTouchBar();

    QAction *addSeparator();
    QAction *addTabBar(QTabBar *tabBar);
    void removeTabBar(QTabBar *tabBar);

    QAction *addMessageBox(QMessageBox *messageBox);
    void removeMessageBox(QMessageBox *messageBox);

    QAction *addDialogButtonBox(QDialogButtonBox *buttonBox);
    void removeDialogButtonBox(QDialogButtonBox *buttonBox);

    void setPrincipialAction(QAction *action);
    QAction *principialAction() const;

    void setEscapeAction(QAction *action);
    QAction *escapeAction() const;

    void setTouchButtonStyle(TouchButtonStyle touchButtonStyle);
    TouchButtonStyle touchButtonStyle() const;

    void clear();

protected:
    bool event(QEvent *event);

private:
    class Private;
    Private *const d;
};

QT_END_NAMESPACE

#endif
