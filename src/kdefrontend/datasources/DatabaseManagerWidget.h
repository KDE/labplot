/***************************************************************************
    File                 : DatabaseManagerWidget.h
    Project              : LabPlot
    Description          : widget for managing database connections
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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
#ifndef DATABASEMANAGERWIDGET_H
#define DATABASEMANAGERWIDGET_H

#include "ui_databasemanagerwidget.h"

class DatabaseManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit DatabaseManagerWidget(QWidget*);
	~DatabaseManagerWidget();

	struct SQLConnection {
		int vendorIndex;
		int port;
		QString name;
		QString hostName;
		QString dbName;
		QString userName;
		QString password;
	};

private:
	Ui::DatabaseManagerWidget ui;
	QString uniqueName();
	QList<SQLConnection*> connectionList;
	bool m_initializing;
	void loadConnection();

private slots:
	void loadSettings();
	void driverChanged();
	void selectFile();
	void addConnection();
	void deleteConnection();
	void connectionChanged(int);
	void testConnection();

	void nameChanged();
	void hostChanged();
	void portChanged();
	void databaseNameChanged();
	void userNameChanged();
	void passwordChanged();
	void saveConnections();
};

#endif
