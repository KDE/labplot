/*
	File		: MQTTTopic.h
	Project		: LabPlot
	Description	: Represents a topic of a MQTTSubscription
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#include "backend/spreadsheet/Spreadsheet.h"

#include <QMqttMessage>

class MQTTSubscription;
class MQTTClient;
class AsciiFilter;

class MQTTTopic : public Spreadsheet {
	Q_OBJECT

public:
	MQTTTopic(const QString& name, MQTTSubscription* subscription, bool loading = false);
	~MQTTTopic() override;

	void setFilter(AsciiFilter*);
	AsciiFilter* filter() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	QString topicName() const;
	MQTTClient* mqttClient() const;
	void newMessage(const QMqttMessage& msg);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();

	QString m_topicName;
	MQTTClient* m_MQTTClient;
	AsciiFilter* m_filter;
	QVector<QMqttMessage> m_messagePuffer;
	QAction* m_plotDataAction;

public Q_SLOTS:
	void read();

private Q_SLOTS:
	void plotData();

Q_SIGNALS:
	void readOccured();
};

#endif // MQTTTOPIC_H
