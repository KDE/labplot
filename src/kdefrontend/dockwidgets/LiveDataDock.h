/*
    File                 : LiveDataDock.h
    Project              : LabPlot
    Description          : Dock widget for live data properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Kovacs Ferencz <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIVEDATADOCK_H
#define LIVEDATADOCK_H

#ifdef HAVE_MQTT
#include <QtMqtt>
#include <QStringList>
#include <QMap>
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/MQTTClient.h"

class MQTTSubscriptionWidget;
#endif

#include <QWidget>
#include <QList>

#include "ui_livedatadock.h"
#include "backend/datasources/LiveDataSource.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

class QTimer;
class QTreeWidgetItem;
class QString;
class QCompleter;

class LiveDataDock : public BaseDock {
	Q_OBJECT

public:
	explicit LiveDataDock(QWidget *parent = nullptr);
	void setLiveDataSource(LiveDataSource* const source);
	~LiveDataDock() override;

private:
	Ui::LiveDataDock ui;
	LiveDataSource* m_liveDataSource{nullptr};

	bool m_paused{false};

	void pauseReading();
	void continueReading();

private slots:
	void nameChanged(const QString&);
	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void sampleSizeChanged(int);
	void updateIntervalChanged(int);
	void keepNValuesChanged(int);

	void updateNow();
	void pauseContinueReading();

#ifdef HAVE_MQTT
public:
	void setMQTTClient(MQTTClient* const client);
	bool testSubscribe(const QString&);
	bool testUnsubscribe(const QString&);

private slots:
	void useWillMessage(bool use);
	void willQoSChanged(int);
	void willRetainChanged(bool);
	void willTopicChanged(const QString &);
	void willMessageTypeChanged(MQTTClient::WillMessageType);
	void willOwnMessageChanged(const QString&);
	void willUpdateTypeChanged(int);
	void willUpdateNow();
	void willUpdateIntervalChanged(int);
    void statisticsChanged(MQTTClient::WillStatisticsType);
	void onMQTTConnect();
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
    void mqttMessageReceivedInBackground(const QByteArray&, const QMqttTopicName&);
	void removeClient(const QString&, quint16);
	void showWillSettings();
    void enableWill(bool enable);

signals:
	void newTopic(const QString&);
    void MQTTClearTopics();
    void updateSubscriptionTree(const QVector<QString>&);

private:
    void addTopicToTree(const QString&);

	struct MQTTHost {
		int count;
		QMqttClient* client;
		QStringList topicList;
		QVector<QString> addedTopics;
	};

	MQTTClient* m_mqttClient{nullptr};
	const MQTTClient* m_previousMQTTClient{nullptr};
	QMap<QPair<QString, int>, MQTTHost> m_hosts;
	MQTTHost* m_currentHost{nullptr};
    MQTTHost* m_previousHost{nullptr};
    bool m_interpretMessage{true};
    MQTTSubscriptionWidget* m_subscriptionWidget;
    QMetaObject::Connection m_updateSubscriptionConn;
#endif
};

#endif // LIVEDATADOCK_H
