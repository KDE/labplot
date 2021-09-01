/*
    File                 : SpreadsheetDock.h
    Project              : LabPlot
    Description          : widget for spreadsheet properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2015 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2012-2013 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETDOCK_H
#define SPREADSHEETDOCK_H
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_spreadsheetdock.h"

class KConfig;
class Spreadsheet;
class AbstractAspect;

class SpreadsheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit SpreadsheetDock(QWidget*);
	void setSpreadsheets(QList<Spreadsheet*>);

private:
	Ui::SpreadsheetDock ui;
	QList<Spreadsheet*> m_spreadsheetList;
	Spreadsheet* m_spreadsheet{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	//SLOTs for changes triggered in SpreadsheetDock
	void rowCountChanged(int);
	void columnCountChanged(int);
	void commentsShownChanged(int);

	//SLOTs for changes triggered in Spreadsheet
	void spreadsheetRowCountChanged(int);
	void spreadsheetColumnCountChanged(int);
	void spreadsheetShowCommentsChanged(int);

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif // SPREADSHEETDOCK_H
