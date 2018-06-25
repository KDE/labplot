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
	void makeChange();
};
#endif

#endif // MQTTERRORWIDGET_H
