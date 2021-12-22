/*
    File                 : MQTTConnectionManagerWidget.h
    Project              : LabPlot
    Description          : widget for managing MQTT connections
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTCONNECTIONMANAGERWIDGET_H
#define MQTTCONNECTIONMANAGERWIDGET_H

#include "ui_mqttconnectionmanagerwidget.h"

class QMqttClient;
class QTimer;

class MQTTConnectionManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTConnectionManagerWidget(QWidget*, const QString&);
	~MQTTConnectionManagerWidget() override;

	struct MQTTConnection {
		QString name;
		int port;
		QString hostName;
		bool useAuthentication;
		QString userName;
		QString password;
		bool useID;
		QString clientID;
		bool retain;
	};

	QString connection() const;
	void setCurrentConnection(const QString&);
	void saveConnections();
	bool checkConnections();

private:
	Ui::MQTTConnectionManagerWidget ui;
	QList<MQTTConnection> m_connections;
	MQTTConnection* m_currentConnection = nullptr;
	bool m_initializing{false};
	QString m_configPath;
	QString m_initConnName;
	QMqttClient* m_client{nullptr};
	bool m_testing{false};
	QTimer* m_testTimer{nullptr};

	QString uniqueName();
	void loadConnection();
	void dataChanged();

private Q_SLOTS:
	void testConnection();
	void loadConnections();
	void addConnection();
	void deleteConnection();
	void connectionChanged(int);
	void nameChanged(const QString&);
	void hostChanged(const QString&);
	void portChanged(const QString&);
	void userNameChanged(const QString&);
	void passwordChanged(const QString&);
	void clientIdChanged(const QString&);
	void authenticationChecked(bool);
	void idChecked(bool);
	void retainChecked(bool);
	void onConnect();
	void onDisconnect();
	void testTimeout();

Q_SIGNALS:
	void changed();
};

#endif	// MQTTCONNECTIONMANAGERWIDGET_H
