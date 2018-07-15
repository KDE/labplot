#ifndef MQTTSUBSCRIPTIONS_H
#define MQTTSUBSCRIPTIONS_H

#include <QVector>
#include <QString>

#include "backend/core/Folder.h"
#include "backend/datasources/MQTTTopic.h"

class MQTTSubscriptions : public Folder{
	Q_OBJECT

public:
	MQTTSubscriptions(const QString& name);
	~MQTTSubscriptions() override;

	void setMQTTClient(MQTTClient *client);
	QString subscriptionName() const;
	void addTopic(const QString&);
	const QVector<MQTTTopic*> topics();
	MQTTClient* mqttClient() const;
	void messageArrived(const QString&, const QString&);
	void topicRead(const QString&);

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

#endif // MQTTSUBSCRIPTIONS_H
