/*
File		: MQTTSubscription.h
Project		: LabPlot
Description	: Represents a subscription made in MQTTClient
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Kovacs Ferencz (kferike98@gmail.com)

SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTSUBSCRIPTION_H
#define MQTTSUBSCRIPTION_H

#include "backend/core/Folder.h"

class MQTTClient;
class MQTTTopic;
class QString;

class MQTTSubscription : public Folder {
	Q_OBJECT

public:
	explicit MQTTSubscription(const QString& name);
	~MQTTSubscription() override;

	void setMQTTClient(MQTTClient*);
	QString subscriptionName() const;
	const QVector<MQTTTopic*> topics() const;
	MQTTClient* mqttClient() const;
	void messageArrived(const QString&, const QString&);

	QIcon icon() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	QString m_subscriptionName;
	MQTTClient* m_MQTTClient{nullptr};

signals:
	void loaded(const QString &);
};

#endif // MQTTSUBSCRIPTION_H
