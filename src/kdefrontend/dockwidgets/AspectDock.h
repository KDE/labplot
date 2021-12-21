/*
    File                 : AspectDock.h
    Project              : LabPlot
    Description          : widget for aspect properties showing name and comments only
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASPECTDOCK_H
#define ASPECTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_aspectdock.h"

class AbstractAspect;
template <class T> class QList;

class AspectDock : public BaseDock {
	Q_OBJECT

public:
	explicit AspectDock(QWidget*);
	void setAspects(QList<AbstractAspect*>);

private:
	Ui::AspectDock ui;

private Q_SLOTS:
	void aspectDescriptionChanged(const AbstractAspect*);
};

#endif // ASPECT_H
