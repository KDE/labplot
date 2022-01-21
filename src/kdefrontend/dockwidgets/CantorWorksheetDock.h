/*
    File                 : CantorWorksheetDock.h
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANTORWORKSHEETDOCK_H
#define CANTORWORKSHEETDOCK_H

#include "ui_cantorworksheetdock.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

class CantorWorksheet;

class CantorWorksheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit CantorWorksheetDock(QWidget*);
	void setCantorWorksheets(QList<CantorWorksheet*>);

private:
	Ui::CantorWorksheetDock ui;
	QList< CantorWorksheet* > m_cantorworksheetlist;
	CantorWorksheet* m_worksheet{nullptr};
	QList<int> index;

	//in the old Cantor the help panel plugin is coming as second
	//in the new code we determine the position via the plugin name
	int m_helpPanelIndex{1};

private Q_SLOTS:
	//SLOTs for changes triggered in WorksheetDock
	//"General"-tab
	void evaluateWorksheet();
	void restartBackend();
	void visibilityRequested();

Q_SIGNALS:
	void info(const QString&);
};

#endif // CANTORWORKSHEETDOCK_H
