/***************************************************************************
File                 : MQTTErrorWidget.h
Project              : LabPlot
Description          : Widget for informing about an MQTT error, and for trying to solve it
--------------------------------------------------------------------
Copyright            : (C) 2018 by Kovacs Ferencz (kferike98@gmail.com)
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

#ifndef MQTTERRORWIDGET_H
#define MQTTERRORWIDGET_H

#ifdef HAVE_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicName>
#include <QtMqtt/QMqttTopicFilter>
#include <QString>
#include <QWidget>

#include "ui_mqtterrorwidget.h"
#include "backend/datasources/MQTTClient.h"

class MQTTErrorWidget : public QWidget {
	Q_OBJECT

public:
	MQTTErrorWidget(QMqttClient::ClientError error = QMqttClient::NoError, MQTTClient* client = 0, QWidget* parent = 0);

private:
	Ui::MQTTErrorWidget ui;
	QMqttClient::ClientError m_error;
	MQTTClient* m_client ;

private slots:
	void tryToReconnect();
};
#endif

#endif // MQTTERRORWIDGET_H
