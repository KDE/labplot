/*
	File                 : MQTTWillSettingsWidget.h
	Project              : LabPlot
	Description          : widget for managing MQTT connection's will settings
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Ferencz Kovacs <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTWILLSETTINGSWIDGET_H
#define MQTTWILLSETTINGSWIDGET_H

#include "ui_mqttwillsettingswidget.h"
#include <QWidget>

#include "backend/datasources/MQTTClient.h"

class MQTTWillSettingsWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTWillSettingsWidget(QWidget*, const MQTTClient::MQTTWill&, const QVector<QString>&);

	MQTTClient::MQTTWill will() const;
	MQTTClient::WillStatisticsType statisticsType() const;

private:
	Ui::MQTTWillSettingsWidget ui;
	MQTTClient::MQTTWill m_will;

	MQTTClient::WillStatisticsType m_statisticsType{MQTTClient::WillStatisticsType::NoStatistics};

Q_SIGNALS:
	void applyClicked();

private Q_SLOTS:
	void enableWillSettings(bool);
	void willMessageTypeChanged(int);
	void loadSettings(const MQTTClient::MQTTWill&, const QVector<QString>&);
	void willUpdateTypeChanged(int);
};

#endif // MQTTWILLSETTINGSWIDGET_H
