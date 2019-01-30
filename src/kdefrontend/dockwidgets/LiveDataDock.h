/***************************************************************************
File                 : LiveDataDock.h
Project              : LabPlot
Description          : Dock widget for live data properties
--------------------------------------------------------------------
Copyright            : (C) 2017 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#ifndef LIVEDATADOCK_H
#define LIVEDATADOCK_H

#ifdef HAVE_MQTT
#include <QtMqtt>
#include <QStringList>
#include <QMap>
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/MQTTClient.h"
#endif

#include <QWidget>
#include <QList>

#include "ui_livedatadock.h"
#include "backend/datasources/LiveDataSource.h"

class QTimer;
class QTreeWidgetItem;
class QString;
class QCompleter;

class LiveDataDock : public QWidget {
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
	void addSubscription();
	void removeSubscription();
	void onMQTTConnect();
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void mqttMessageReceivedInBackground(const QByteArray&, const QMqttTopicName&);
	void setTopicCompleter(const QString&);
	void topicTimeout();
	void fillSubscriptions();
	void scrollToTopicTreeItem(const QString&);
	void scrollToSubsriptionTreeItem(const QString&);
	void removeClient(const QString&, quint16);
	void showWillSettings();

signals:
	void newTopic(const QString&);

private:
	void updateSubscriptionCompleter();
	void addTopicToTree(const QString&);
	void manageCommonLevelSubscriptions();

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
	QCompleter* m_topicCompleter{nullptr};
	QCompleter* m_subscriptionCompleter{nullptr};
	bool m_searching{true};
	QTimer* m_searchTimer;
	bool m_interpretMessage{true};
	QString m_mqttUnsubscribeName;
#endif
};

#endif // LIVEDATADOCK_H
