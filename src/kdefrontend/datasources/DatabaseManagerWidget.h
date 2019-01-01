/***************************************************************************
    File                 : DatabaseManagerWidget.h
    Project              : LabPlot
    Description          : widget for managing database connections
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

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
	explicit DatabaseManagerWidget(QWidget*, QString);

	struct SQLConnection {
		int port;
		QString name;
		QString driver;
		QString hostName;
		QString dbName;
		QString userName;
		QString password;
		bool customConnectionEnabled{false};
		QString customConnectionString;
	};

	QString connection() const;
	void setCurrentConnection(const QString&);
	void saveConnections();
	static bool isFileDB(const QString&);
	static bool isODBC(const QString&);

private:
	Ui::DatabaseManagerWidget ui;
	QList<SQLConnection> m_connections;
	bool m_initializing{false};
	QString m_configPath;
	QString m_initConnName;

	QString uniqueName();
	void loadConnection();
	int defaultPort(const QString&) const;
	void dataChanged();

private slots:
	void loadConnections();
	void addConnection();
	void deleteConnection();
	void testConnection();
	void connectionChanged(int);

	void nameChanged(const QString&);
	void driverChanged();
	void selectFile();
	void hostChanged();
	void portChanged();
	void databaseNameChanged();
	void customConnectionEnabledChanged(int);
	void customConnectionChanged();
	void userNameChanged();
	void passwordChanged();

signals:
	void changed();
};

#endif
