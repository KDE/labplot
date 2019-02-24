/***************************************************************************
File		: MQTTSubscription.h
Project		: LabPlot
Description	: Represents a subscription made in MQTTClient
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

#ifndef MQTTSUBSCRIPTION_H
#define MQTTSUBSCRIPTION_H

#include "backend/core/Folder.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTTopic.h"

class QString;
#endif

class MQTTSubscription : public Folder {
#ifdef HAVE_MQTT
	Q_OBJECT

public:
	explicit MQTTSubscription(const QString& name);
	~MQTTSubscription() override;

	void setMQTTClient(MQTTClient *client);
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

#endif //HAVE_MQTT
};

#endif // MQTTSUBSCRIPTION_H
