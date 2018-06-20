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
#endif

class LiveDataDock : public QWidget {
	Q_OBJECT

public:
	explicit LiveDataDock(QWidget *parent = 0);
	void setLiveDataSources(const QList<LiveDataSource*>& sources);
	~LiveDataDock();

private:
	Ui::LiveDataDock ui;
	QList<LiveDataSource*> m_liveDataSources;

	bool m_paused;

	void pauseReading();
	void continueReading();

#ifdef HAVE_MQTT
	QMqttClient* m_client;
	QCompleter *m_completer;
	QStringList m_topicList;
	bool m_editing;
	QTimer *m_timer;
#endif

private slots:

	void updateTypeChanged(int);
	void readingTypeChanged(int);
	void sampleRateChanged(int);
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
	void addSubscription(const QString&, quint16);
	void onMQTTConnect();
	void mqttMessageReceived(const QByteArray&, const QMqttTopicName&);
	void setCompleter(const QString&);
	void topicBeingTyped(const QString&);
	void topicTimeout();
	void fillSubscriptions();
#endif

public slots:

signals:
#ifdef HAVE_MQTT
	void newTopic(const QString&);
#endif

};

#endif // LIVEDATADOCK_H
