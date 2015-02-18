/***************************************************************************
    File                 : SpreadsheetDock.h
    Project              : LabPlot
    Description          : widget for spreadsheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2014 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef SPREADSHEETDOCK_H
#define SPREADSHEETDOCK_H

#include <QtWidgets/QWidget>
#include <QtCore/QList>
#include <kconfig.h>
#include "ui_spreadsheetdock.h"

class Spreadsheet;
class AbstractAspect;

class SpreadsheetDock: public QWidget{
	Q_OBJECT

public:
	explicit SpreadsheetDock(QWidget *parent);
	void setSpreadsheets(QList<Spreadsheet*>);

private:
	Ui::SpreadsheetDock ui;
	QList<Spreadsheet*> m_spreadsheetList;
	Spreadsheet* m_spreadsheet;
	bool m_initializing;

private slots:
	//SLOTs for changes triggered in WorksheetDock
	void nameChanged();
	void commentChanged();
	void rowCountChanged(int);
	void columnCountChanged(int);
	void commentsShownChanged(int);

	//SLOTs for changes triggered in Spreadsheet
	void spreadsheetDescriptionChanged(const AbstractAspect*);
	void spreadsheetRowCountChanged(int);
	void spreadsheetColumnCountChanged(int);
	void spreadsheetShowCommentsChanged(int);

	void loadConfigFromTemplate(KConfig&);
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);

signals:
	void info(const QString&);
};

#endif // SPREADSHEETDOCK_H
