/***************************************************************************
File                 : MQTTErrorWidget.cpp
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

#include "src/kdefrontend/datasources/MQTTErrorWidget.h"
#ifdef HAVE_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicFilter>
#include <QtMqtt/QMqttMessage>

MQTTErrorWidget::MQTTErrorWidget(QMqttClient::ClientError error, MQTTClient* client, QWidget *parent) : QWidget(parent),	
	m_error(error),
	m_client(client)
{
	ui.setupUi(this);
	bool close = false;
	//showing the appropriate options according to the error type
	switch (m_error) {
	case QMqttClient::ClientError::IdRejected:
		ui.lePassword->hide();
		ui.lPassword->hide();
		ui.leUserName->hide();
		ui.lUserName->hide();
		ui.lErrorType->setText("The client ID is malformed. This might be related to its length.\nSet new ID!");
		break;
	case QMqttClient::ClientError::BadUsernameOrPassword:
		ui.lId->hide();
		ui.leId->hide();
		ui.lErrorType->setText("The data in the username or password is malformed.\nSet new username and password!");
		break;
	case QMqttClient::ClientError::NotAuthorized:
		ui.lId->hide();
		ui.leId->hide();
		ui.lErrorType->setText("The client is not authorized to connect.");
		break;
	case QMqttClient::ClientError::ServerUnavailable:
		ui.lePassword->hide();
		ui.lPassword->hide();
		ui.leUserName->hide();
		ui.lUserName->hide();
		ui.lErrorType->setText("The network connection has been established, but the service is unavailable on the broker side.");
		break;
	case QMqttClient::ClientError::UnknownError:
		ui.lePassword->hide();
		ui.lPassword->hide();
		ui.leUserName->hide();
		ui.lUserName->hide();
		ui.lErrorType->setText("An unknown error occurred.");
		break;
	case QMqttClient::NoError:
	case QMqttClient::InvalidProtocolVersion:
	case QMqttClient::TransportInvalid:
	case QMqttClient::ProtocolViolation:
		close = true;
		break;
	default:
		close = true;
		break;
	}
	connect(ui.bChange, &QPushButton::clicked, this, &MQTTErrorWidget::tryToReconnect);
	setAttribute(Qt::WA_DeleteOnClose);
	if (close)
		this->close();
}

/*!
 *\brief Try to reconnect in MQTTClient after reseting options that might cause the error
 */
void MQTTErrorWidget::tryToReconnect(){
	bool ok = false;
	switch (m_error) {
	case QMqttClient::ClientError::IdRejected:
		if (!ui.leId->text().isEmpty()) {
			m_client->setMQTTClientId(ui.leId->text());
			m_client->read();
			ok = true;
		}
		break;
	case QMqttClient::ClientError::BadUsernameOrPassword:
		if (!ui.lePassword->text().isEmpty() && !ui.leUserName->text().isEmpty()) {
			m_client->setMQTTClientAuthentication(ui.leUserName->text(), ui.lePassword->text());
			m_client->read();
			ok = true;
		}
		break;
	case QMqttClient::ClientError::NotAuthorized:
		if (!ui.lePassword->text().isEmpty() && !ui.leUserName->text().isEmpty()) {
			m_client->setMQTTClientAuthentication(ui.leUserName->text(), ui.lePassword->text());
			m_client->read();
			ok = true;
		}
		break;
	case QMqttClient::NoError:
	case QMqttClient::InvalidProtocolVersion:
	case QMqttClient::TransportInvalid:
	case QMqttClient::ServerUnavailable:
	case QMqttClient::UnknownError:
	case QMqttClient::ProtocolViolation:
		break;
	default:
		break;
	}
	if (ok)
		this->close();
}
#endif
