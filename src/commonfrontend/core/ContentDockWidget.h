/*
	File                 : PartMdiView.h
	Project              : LabPlot
	Description          : QMdiSubWindow wrapper for aspect views.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2013-2019 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007, 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2007, 2008 Knut Franke <knut.franke@gmx.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTENTDOCKWIDGET_H
#define CONTENTDOCKWIDGET_H

#include <DockWidget.h>

class AbstractAspect;
class AbstractPart;

class ContentDockWidget : public ads::CDockWidget {
	Q_OBJECT

public:
	explicit ContentDockWidget(AbstractPart*);
	~ContentDockWidget() override;
	AbstractPart* part() const;
	void suppressDeletion(bool);

private:
	AbstractPart* m_part;
	bool m_closing{false};

private Q_SLOTS:
	void handleAspectDescriptionChanged(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void slotWindowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);
};

#endif // ifndef CONTENTDOCKWIDGET_H
