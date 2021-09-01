/*
File		: MQTTTopic.h
Project		: LabPlot
Description	: Represents a topic of a MQTTSubscription
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Kovacs Ferencz (kferike98@gmail.com)

*/

/***************************************************************************
*                                                                         *
*  SPDX-License-Identifier: GPL-2.0-or-later
*                                                                         *
***************************************************************************/

#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#include "backend/spreadsheet/Spreadsheet.h"

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
	void newMessage(const QString&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();

	QString m_topicName;
	MQTTClient* m_MQTTClient;
	AsciiFilter* m_filter;
	QVector<QString> m_messagePuffer;
	QAction* m_plotDataAction;

public slots:
	void read();

private slots:
	void plotData();

signals:
	void readOccured();
};

#endif // MQTTTOPIC_H
