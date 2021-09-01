/*
File                 : MQTTErrorWidget.h
Project              : LabPlot
Description          : Widget for informing about an MQTT error, and for trying to solve it
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTERRORWIDGET_H
#define MQTTERRORWIDGET_H

#include <QMqttClient>
#include "ui_mqtterrorwidget.h"

class MQTTClient;

class MQTTErrorWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTErrorWidget(QMqttClient::ClientError error = QMqttClient::NoError, MQTTClient* client = nullptr, QWidget* parent = nullptr);

private:
	Ui::MQTTErrorWidget ui;
	QMqttClient::ClientError m_error;
	MQTTClient* m_client;

private slots:
	void tryToReconnect();
};

#endif // MQTTERRORWIDGET_H
