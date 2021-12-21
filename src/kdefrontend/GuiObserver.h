/*
    File                 : GuiObserver.h
    Project              : LabPlot
    Description          : GUI observer
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GUIOBSERVER_H
#define GUIOBSERVER_H

#include <QObject>

class MainWin;
class AbstractAspect;

class GuiObserver : public QObject {
	Q_OBJECT

public:
	explicit GuiObserver(MainWin*);

private:
	MainWin* m_mainWindow;

private Q_SLOTS:
	void selectedAspectsChanged(QList<AbstractAspect*>&) const;
	void hiddenAspectSelected(const AbstractAspect*) const;
};

#endif
