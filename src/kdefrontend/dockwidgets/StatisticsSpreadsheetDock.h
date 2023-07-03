/*
	File                 : StatisticsSpreadsheetDock.h
	Project              : LabPlot
	Description          : widget for statistics spreadsheet properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATISTICSSPREADSHEETDOCK_H
#define STATISTICSSPREADSHEETDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_statisticsspreadsheetdock.h"

class KConfig;
class StatisticsSpreadsheet;

class StatisticsSpreadsheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit StatisticsSpreadsheetDock(QWidget*);
	void setSpreadsheets(QList<StatisticsSpreadsheet*>);

private:
	Ui::StatisticsSpreadsheetDock ui;
	QList<StatisticsSpreadsheet*> m_spreadsheetList;
	StatisticsSpreadsheet* m_spreadsheet{nullptr};

	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	// SLOTs for changes triggered in StatisticsSpreadsheetDock

	// SLOTs for changes triggered in Spreadsheet

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // STATISTICSSPREADSHEETDOCK_H
