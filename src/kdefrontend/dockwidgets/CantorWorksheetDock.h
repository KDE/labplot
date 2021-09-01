/*
    File                 : CantorWorksheetDock.h
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri (garvitdelhi@gmail.com)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef CANTORWORKSHEETDOCK_H
#define CANTORWORKSHEETDOCK_H

#include "ui_cantorworksheetdock.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <3rdparty/cantor/panelplugin.h>

#include <QPair>
#include <QWidget>

class CantorWorksheet;
class AbstractAspect;

class CantorWorksheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit CantorWorksheetDock(QWidget *parent);
	void setCantorWorksheets(QList<CantorWorksheet*>);

private:
	Ui::CantorWorksheetDock ui;
	QList< CantorWorksheet* > m_cantorworksheetlist;
	CantorWorksheet* m_worksheet{nullptr};
	QList<int> index;

private slots:
	//SLOTs for changes triggered in WorksheetDock
	//"General"-tab
	void evaluateWorksheet();
	void restartBackend();
	void visibilityRequested();

signals:
	void info(const QString&);

};

#endif // CANTORWORKSHEETDOCK_H
