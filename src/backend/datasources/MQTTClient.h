/***************************************************************************
File		: MQTTClient.h
Project		: LabPlot
Description	: Represents a MQTT Client
--------------------------------------------------------------------
Copyright	: (C) 2018 Kovacs Ferencz (kferike98@gmail.com)

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
#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "backend/core/Folder.h"

#ifdef HAVE_MQTT
#include <QTimer>
#include <QVector>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicFilter>
#include <QtMqtt/QMqttTopicName>
#include <QMap>

class QString;
class AbstractFileFilter;
class MQTTSubscription;
class QAction;
#endif

class MQTTClient : public Folder{
#ifdef HAVE_MQTT
	Q_OBJECT

public:	
	enum UpdateType {
		TimeInterval = 0,
		NewData
	};

	enum ReadingType {
		ContinuousFixed = 0,
		FromEnd,
		TillEnd
	};

	enum WillMessageType {
		OwnMessage = 0,
		Statistics,
		LastMessage
	};

	enum WillUpdateType {
		TimePeriod = 0,
		OnClick
	};

	enum WillStatistics {
		Minimum = 0,
		Maximum,
		ArithmeticMean,
		GeometricMean,
		HarmonicMean,
		ContraharmonicMean,
		Median,
		Variance,
		StandardDeviation,
		MeanDeviation,
		MeanDeviationAroundMedian,
		MedianDeviation,
		Skewness,
		Kurtosis,
		Entropy
	};

	explicit MQTTClient(const QString& name);
	~MQTTClient() override;

	void ready();

	UpdateType updateType() const;
	void setUpdateType(UpdateType);

	ReadingType readingType() const;
	void setReadingType(ReadingType);

	int sampleSize() const;
	void setSampleSize(int);

	bool isPaused() const;

	void setUpdateInterval(int);
	int updateInterval() const;

	void setKeepNValues(int);
	int keepNValues() const;

	void setKeepLastValues(bool);
	bool keepLastValues() const;

	void setMQTTClientHostPort(const QString&, const quint16&);
	void setMQTTClientAuthentication(const QString&, const QString&);
	void setMQTTClientId(const QString&);

	void addInitialMQTTSubscriptions(const QMqttTopicFilter&, const quint8&);
	QVector<QString> MQTTSubscriptions() const;

	bool checkTopicContains(const QString& superior, const QString& inferior);
	QString checkCommonLevel(const QString& first, const QString& second);

	QString clientHostName() const;
	quint16 clientPort() const;
	QString clientPassword() const;
	QString clientUserName() const;
	QString clientID () const;

	void updateNow();
	void pauseReading();
	void continueReading();

	void setFilter(AbstractFileFilter*);
	AbstractFileFilter* filter() const;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QVector<QString> topicNames() const;
	bool checkAllArrived();

	void setMQTTWillUse(bool);
	bool MQTTWillUse() const;

	void setWillTopic(const QString&);
	QString willTopic() const;

	void setWillRetain(bool);
	bool willRetain() const;

	void setWillQoS(quint8);
	quint8 willQoS() const;

	void setWillMessageType(WillMessageType);
	WillMessageType willMessageType() const;

	void setWillOwnMessage(const QString&);
	QString willOwnMessage() const;

	WillUpdateType willUpdateType() const;
	void setWillUpdateType(WillUpdateType);

	int willTimeInterval() const;
	void setWillTimeInterval(int);

	void startWillTimer() const;
	void stopWillTimer() const;

	void updateWillMessage() ;

	void setMQTTRetain(bool);
	bool MQTTRetain() const;

	void setMQTTUseID(bool);
	bool MQTTUseID() const;

	void setMQTTUseAuthentication(bool);
	bool MQTTUseAuthentication() const;

	void clearLastMessage();
	void addWillStatistics(WillStatistics);
	void removeWillStatistics(WillStatistics);
	QVector<bool> willStatistics() const;

	void addMQTTSubscription(const QString&, quint8);
	void removeMQTTSubscription(const QString&);
	void addBeforeRemoveSubscription(const QString&, quint8);
	void reparentTopic(const QString& topic, const QString& parent);

private:

	UpdateType m_updateType;
	ReadingType m_readingType;
	bool m_paused;
	bool m_prepared;
	int m_sampleSize;
	int m_keepNValues;
	int m_updateInterval;
	AbstractFileFilter* m_filter;
	QTimer* m_updateTimer;
	QMqttClient* m_client;
	QMap<QMqttTopicFilter, quint8> m_subscribedTopicNameQoS;
	QVector<QString> m_subscriptions;
	QVector<QString> m_topicNames;
	bool m_MQTTTest;
	bool m_MQTTUseWill;
	QString m_willMessage;
	QString m_willTopic;
	bool m_willRetain;
	quint8 m_willQoS;
	WillMessageType m_willMessageType;
	QString m_willOwnMessage;
	QString m_willLastMessage;
	QTimer* m_willTimer;
	int m_willTimeInterval;
	WillUpdateType m_willUpdateType;
	QVector<bool> m_willStatistics;
	bool m_MQTTFirstConnectEstablished;
	bool m_MQTTRetain;
	bool m_MQTTUseID;
	bool m_MQTTUseAuthentication;
	QVector<MQTTSubscription*> m_MQTTSubscriptions;
	bool m_disconnectForWill;
	bool m_loaded;
	int m_subscriptionsLoaded;
	int m_subscriptionCountToLoad;

public slots:
	void read();

private slots:
	void onMQTTConnect();
	void MQTTSubscriptionMessageReceived(const QMqttMessage&);
	void MQTTErrorChanged(QMqttClient::ClientError);
	void subscriptionLoaded(const QString&);

signals:
	void MQTTSubscribed();
	void MQTTTopicsChanged();
	void readFromTopics();
	void clientAboutToBeDeleted(const QString&);
#endif //HAVE_MQTT
};
#endif // MQTTCLIENT_H
