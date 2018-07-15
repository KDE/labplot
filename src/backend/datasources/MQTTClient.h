#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H
#ifdef HAVE_MQTT

#include "backend/core/Folder.h"

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
class MQTTSubscriptions;
class QAction;

class MQTTClient : public Folder{
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
	virtual ~MQTTClient() override;

	void ready();

	UpdateType updateType() const;
	void setUpdateType(UpdateType);

	ReadingType readingType() const;
	void setReadingType(ReadingType);

	int sampleRate() const;
	void setSampleRate(int);

	bool isPaused() const;

	void setUpdateInterval(int);
	int updateInterval() const;

	void setKeepNvalues(int);
	int keepNvalues() const;

	void setKeepLastValues(bool);
	bool keepLastValues() const;

	void setMqttClientHostPort(const QString&, const quint16&);
	void setMqttClientAuthentication(const QString&, const QString&);
	void setMqttClientId(const QString&);
	QMqttClient mqttClient() const;

	void addMqttSubscriptions(const QMqttTopicFilter&, const quint8&);
	QVector<QString> mqttSubscribtions() const;

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
	/*QMenu* createContextMenu() override;
		QWidget* view() const override;*/

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	int topicNumber();
	int topicIndex(const QString&);
	QVector<QString> topicNames() const;
	bool checkAllArrived();

	void setMqttWillUse(bool);
	bool mqttWillUse() const;

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

	void setWillForMqtt() ;

	void setMqttRetain(bool);
	bool mqttRetain() const;

	void setMQTTUseID(bool);
	bool mqttUseID() const;

	void setMQTTUseAuthentication(bool);
	bool mqttUseAuthentication() const;

	void clearLastMessage();
	void addWillStatistics(WillStatistics);
	void removeWillStatistics(WillStatistics);
	QVector<bool> willStatistics() const;

	void newMQTTSubscription(const QString&, quint8);
	void removeMQTTSubscription(const QString&);
	void addBeforeRemoveSubscription(const QString&, quint8);
	void reparentTopic(const QString& topic, const QString& parent);

private:
	//void initActions();

	UpdateType m_updateType;
	ReadingType m_readingType;

	bool m_paused;
	bool m_prepared;
	bool m_keepLastValues;

	int m_sampleRate;
	int m_keepNvalues;
	int m_updateInterval;

	AbstractFileFilter* m_filter;

	QTimer* m_updateTimer;
	/*
		QAction* m_reloadAction;
		QAction* m_toggleLinkAction;
		QAction* m_showEditorAction;
		QAction* m_showSpreadsheetAction;
		QAction* m_plotDataAction;*/

	QMqttClient* m_client;
	QMap<QMqttTopicFilter, quint8> m_subscribedTopicNameQoS;
	QVector<QString> m_subscriptions;
	QVector<QString> m_topicNames;
	bool m_mqttTest;
	bool m_mqttUseWill;
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
	bool m_mqttFirstConnectEstablished;
	bool m_mqttRetain;
	bool m_mqttUseID;
	bool m_mqttUseAuthentication;
	QVector<MQTTSubscriptions*> m_mqttSubscriptions;
	bool m_disconnectForWill;
	bool m_loaded;
	int m_subscriptionsLoaded;
	int m_subscriptionCount;

public slots:
	void read();

private slots:
	void onMqttConnect();
	void mqttSubscribtionMessageReceived(const QMqttMessage&);
	void mqttErrorChanged(QMqttClient::ClientError);
	void subscriptionLoaded(const QString&);

signals:
	void mqttSubscribed();
	void mqttNewTopicArrived();
	void readFromTopics();
	void clientAboutToBeDeleted(const QString&);
};

#endif
#endif // MQTTCLIENT_H
