#ifndef MQTTSUBSCRIPTION_H
#define MQTTSUBSCRIPTION_H

#include <QVector>
#include <QString>

#include "backend/core/Folder.h"
#include "backend/datasources/MQTTTopic.h"

class MQTTSubscription : public Folder{
	Q_OBJECT

public:
	MQTTSubscription(const QString& name);
	~MQTTSubscription() override;

	void setMQTTClient(MQTTClient *client);
	QString subscriptionName() const;
	void addTopic(const QString&);
	const QVector<MQTTTopic*> topics();
	MQTTClient* mqttClient() const;
	void messageArrived(const QString&, const QString&);

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	QString m_subscriptionName;
	MQTTClient* m_MQTTClient;
	QVector<MQTTTopic*> m_topics;

public slots:

private slots:

signals:
	void loaded(const QString &);
};

#endif // MQTTSUBSCRIPTION_H
