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

#include <QWidget>

#include "ui_livedatadock.h"
#include "backend/datasources/LiveDataSource.h"
#include <QList>
#ifdef HAVE_MQTT
#include <QtMqtt>
#include <QCompleter>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTreeWidgetItem>
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/MQTTClient.h"
#endif

class LiveDataDock : public QWidget {
	Q_OBJECT

public:
	explicit LiveDataDock(QWidget *parent = 0);
	void setLiveDataSources(const QList<LiveDataSource*>& sources);
#ifdef HAVE_MQTT
	void setMQTTClients(const QList<MQTTClient*>& clients);
	bool checkTopicContains(const QString& superior, const QString& inferior);
#endif
	~LiveDataDock();

private:
	Ui::LiveDataDock ui;
	QList<LiveDataSource*> m_liveDataSources;

	bool m_paused;

	void pauseReading();
	void continueReading();

#ifdef HAVE_MQTT
	QList<MQTTClient*> m_mqttClients;
	QMqttClient* m_client;
	QCompleter *m_completer;
	QStringList m_topicList;
	bool m_editing;
	QTimer *m_timer;
	QTimer *m_messageTimer;
	bool m_interpretMessage;
	bool m_MQTTUsed;
	const MQTTClient* m_previousMQTTClient;
	bool m_mqttSubscribeButton;
	QString m_mqttUnsubscribeName;
	QVector<QString> m_addedTopics;
#endif

private slots:

	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void sampleSizeChanged(int);
	void updateIntervalChanged(int);
	void keepNvaluesChanged(const QString&);

	void updateNow();
	void pauseContinueReading();

#ifdef HAVE_MQTT
	void useWillMessage(int);
	void willQoSChanged(int);
	void willRetainChanged(int);
	void willTopicChanged(const QString &);
	void willMessageTypeChanged(int);
	void willOwnMessageChanged(const QString&);
	void updateTopics();
	void willUpdateChanged(int);
	void willUpdateNow();
	void willUpdateIntervalChanged(const QString&);
	void statisticsChanged(QListWidgetItem *);
	void addSubscription();
	void onMQTTConnect();
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void setCompleter(const QString&);
	void topicTimeout();
	void fillSubscriptions();
	void stopStartReceive();
	void mqttButtonSubscribe(QTreeWidgetItem *, int);
	void mqttButtonUnsubscribe(QListWidgetItem *);
	void searchTreeItem(const QString& rootName);
#endif

public slots:

signals:
#ifdef HAVE_MQTT
	void newTopic(const QString&);
#endif

};

#endif // LIVEDATADOCK_H
