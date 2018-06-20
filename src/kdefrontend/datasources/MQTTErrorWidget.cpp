#include "src/kdefrontend/datasources/MQTTErrorWidget.h"

#ifdef HAVE_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/QMqttTopicFilter>
#include <QtMqtt/QMqttMessage>
#include <QMessageBox>

MQTTErrorWidget::MQTTErrorWidget(QMqttClient::ClientError error, LiveDataSource * source, QWidget *parent) : QWidget(parent),
	m_source(source),
	m_error(error)
{
	ui.setupUi(this);
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
	default:
		break;
	}
	connect(ui.bChange, &QPushButton::clicked, this, &MQTTErrorWidget::makeChange);

	setAttribute(Qt::WA_DeleteOnClose);
}

void MQTTErrorWidget::makeChange(){
	bool ok = false;
	switch (m_error) {
	case QMqttClient::ClientError::IdRejected:
		if(!ui.leId->text().isEmpty()) {
			m_source->setMqttClientId(ui.leId->text());
			m_source->read();
			ok = true;
		}
		break;
	case QMqttClient::ClientError::BadUsernameOrPassword:
		if(!ui.lePassword->text().isEmpty() && !ui.leUserName->text().isEmpty()) {
			m_source->setMqttClientAuthentication(ui.leUserName->text(), ui.lePassword->text());
			m_source->read();
			ok = true;
		}
		break;
	case QMqttClient::ClientError::NotAuthorized:
		if(!ui.lePassword->text().isEmpty() && !ui.leUserName->text().isEmpty()) {
			m_source->setMqttClientAuthentication(ui.leUserName->text(), ui.lePassword->text());
			m_source->read();
			ok = true;
		}
		break;
	default:
		break;
	}
	if (ok)
		this->close();
}
#endif
