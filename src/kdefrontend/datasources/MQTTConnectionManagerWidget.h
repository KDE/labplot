/***************************************************************************
	File                 : MQTTConnectionManagerWidget.h
	Project              : LabPlot
	Description          : widget for managing MQTT connections
	--------------------------------------------------------------------
	Copyright            : (C) 2018 Ferencz Kovacs (kferike98@gmail.com)

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
#ifndef MQTTCONNECTIONMANAGERWIDGET_H
#define MQTTCONNECTIONMANAGERWIDGET_H

#include "ui_mqttconnectionmanagerwidget.h"

class MQTTConnectionManagerWidget : public QWidget {
#ifdef HAVE_MQTT
	Q_OBJECT

public:
	explicit MQTTConnectionManagerWidget(QWidget*, const QString&);

	struct MQTTConnection {
		int port;
		QString hostName;
		bool useAuthentication;
		QString userName;
		QString password;
		bool useID;
		QString clientID;
	};

	QString connection() const;
	void setCurrentConnection(const QString&);
	void saveConnections();
	bool checkConnections();

private:
	Ui::MQTTConnectionManagerWidget ui;
	QList<MQTTConnection> m_connections;
	bool m_initializing;
	QString m_configPath;
	QString m_initConnName;

	QString uniqueName();
	void loadConnection();
	void dataChanged();

private slots:
	void loadConnections();
	void addConnection();
	void deleteConnection();
	void connectionChanged(int);
	void hostChanged(const QString&);
	void portChanged();
	void userNameChanged(const QString&);
	void passwordChanged(const QString&);
	void clientIdChanged(const QString&);
	void authenticationChecked(int);
	void idChecked(int);

signals:
	void changed();
#endif
};

#endif
