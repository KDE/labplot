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
	void setLiveDataSources(const QList<LiveDataSource*>& sources);
	~LiveDataDock() override;

private:
	Ui::LiveDataDock ui;
	QList<LiveDataSource*> m_liveDataSources;

	bool m_paused;

	void pauseReading();
	void continueReading();

private slots:
	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void sampleSizeChanged(int);
	void updateIntervalChanged(int);
	void keepNValuesChanged(int);

	void updateNow();
	void pauseContinueReading();

#ifdef HAVE_MQTT
public:
	void setMQTTClients(const QList<MQTTClient*>& clients);
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
	void removeClient(const QString&);
	void showWillSettings();

signals:
	void newTopic(const QString&);

private:
	void updateSubscriptionCompleter();
	void addTopicToTree(const QString&);
	bool checkTopicContains(const QString& superior, const QString& inferior);
	QString checkCommonLevel(const QString& first, const QString& second);
	void findSubscriptionLeafChildren(QVector<QTreeWidgetItem *>&, QTreeWidgetItem*);
	int checkCommonChildCount(int levelIdx, int level, QStringList& namelist, QTreeWidgetItem* currentItem);
	void manageCommonLevelSubscriptions();
	int commonLevelIndex(const QString& first, const QString& second);
	void addSubscriptionChildren(QTreeWidgetItem* topic, QTreeWidgetItem* subscription);
	void restoreSubscriptionChildren(QTreeWidgetItem* topic, QTreeWidgetItem* subscription, const QStringList&, int level);

	QList<MQTTClient*> m_mqttClients;
	QMap<QString, QMqttClient*> m_clients;
	QCompleter* m_topicCompleter;
	QCompleter* m_subscriptionCompleter;
	QMap<QString, QStringList> m_topicList;
	bool m_searching;
	QTimer* m_searchTimer;
	bool m_interpretMessage;
	const MQTTClient* m_previousMQTTClient;
	QString m_mqttUnsubscribeName;
	QMap<QString, QVector<QString>> m_addedTopics;
#endif
};

#endif // LIVEDATADOCK_H
