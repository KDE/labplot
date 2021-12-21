/*
    File		: MQTTClient.h
    Project		: LabPlot
    Description	: Represents a MQTT Client
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "backend/core/Folder.h"

#include <QVector>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicFilter>
#include <QtMqtt/QMqttTopicName>
#include <QMap>

class AsciiFilter;
class MQTTSubscription;
class MQTTTopic;
class QAction;
class QTimer;
class QString;

class MQTTClient : public Folder {
	Q_OBJECT

public:
	enum class UpdateType {
		TimeInterval = 0,
		NewData
	};

	enum class ReadingType {
		ContinuousFixed = 0,
		FromEnd,
		TillEnd
	};

	enum class WillMessageType {
		OwnMessage = 0,
		Statistics,
		LastMessage
	};

	enum class WillUpdateType {
		TimePeriod = 0,
		OnClick
	};

	enum class WillStatisticsType {
		NoStatistics = -1,
		Minimum,
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

	struct MQTTWill {
		bool enabled{false};
		QString willMessage;
		QString willTopic;
		bool willRetain{false};
		quint8 willQoS{0};
		WillMessageType willMessageType{MQTTClient::WillMessageType::OwnMessage};
		QString willOwnMessage;
		QString willLastMessage;
		int willTimeInterval{1000};
		WillUpdateType willUpdateType{MQTTClient::WillUpdateType::TimePeriod};
		QVector<bool> willStatistics{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	};

	explicit MQTTClient(const QString& name);
	virtual ~MQTTClient() override;

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

	void setMQTTClientHostPort(const QString&, quint16);
	void setMQTTClientAuthentication(const QString&, const QString&);
	void setMQTTClientId(const QString&);

	void addInitialMQTTSubscriptions(const QMqttTopicFilter&, quint8);
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

	void setFilter(AsciiFilter*);
	AsciiFilter* filter() const;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QVector<QString> topicNames() const;
	bool checkAllArrived();

	void setWillSettings(const MQTTWill&);
	MQTTWill willSettings() const;

	void setMQTTWillUse(bool);
	bool MQTTWillUse() const;

	void setWillTopic(const QString&);
	QString willTopic() const;
	QString statistics(const MQTTTopic*) const;

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
	void addWillStatistics(WillStatisticsType);
	void removeWillStatistics(WillStatisticsType);
	QVector<bool> willStatistics() const;

	void addMQTTSubscription(const QString&, quint8);
	void removeMQTTSubscription(const QString&);
	void addBeforeRemoveSubscription(const QString&, quint8);
	void reparentTopic(const QString& topic, const QString& parent);

private:
	UpdateType m_updateType{UpdateType::TimeInterval};
	ReadingType m_readingType{ReadingType::ContinuousFixed};
	bool m_paused{false};
	bool m_prepared{false};
	int m_sampleSize{1};
	int m_keepNValues{0};
	int m_updateInterval{1000};
	AsciiFilter* m_filter{nullptr};
	QTimer* m_updateTimer;
	QMqttClient* m_client;
	QMap<QMqttTopicFilter, quint8> m_subscribedTopicNameQoS;
	QVector<QString> m_subscriptions;
	QVector<QString> m_topicNames;
	bool m_MQTTTest{false};
	QTimer* m_willTimer;
	bool m_MQTTFirstConnectEstablished{false};
	bool m_MQTTRetain{false};
	bool m_MQTTUseID{false};
	bool m_MQTTUseAuthentication{false};
	QVector<MQTTSubscription*> m_MQTTSubscriptions;
	bool m_disconnectForWill{false};
	bool m_loaded{false};
	int m_subscriptionsLoaded{0};
	int m_subscriptionCountToLoad{0};
	MQTTWill m_MQTTWill;

public Q_SLOTS:
	void read();

private Q_SLOTS:
	void onMQTTConnect();
	void MQTTSubscriptionMessageReceived(const QMqttMessage&);
	void MQTTErrorChanged(QMqttClient::ClientError);
	void subscriptionLoaded(const QString&);

Q_SIGNALS:
	void MQTTSubscribed();
	void MQTTTopicsChanged();
	void readFromTopics();
	void clientAboutToBeDeleted(const QString&, quint16);
};
#endif // MQTTCLIENT_H
