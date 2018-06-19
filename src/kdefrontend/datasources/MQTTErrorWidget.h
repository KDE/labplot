#ifndef MQTTERRORWIDGET_H
#define MQTTERRORWIDGET_H

#include "ui_mqtterrorwidget.h"
#include "backend/datasources/LiveDataSource.h"
#include <QString>
#include <QWidget>

#ifdef HAVE_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/qmqttclient.h>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/qmqttsubscription.h>
#include <QtMqtt/QMqttTopicName>
#include <QtMqtt/QMqttTopicFilter>


class MQTTErrorWidget : public QWidget {
	Q_OBJECT

public:
	MQTTErrorWidget(QWidget* parent = 0, QMqttClient::ClientError errorType = QMqttClient::NoError, LiveDataSource * source = 0);
	~MQTTErrorWidget();


private:
	Ui::MQTTErrorWidget ui;
#ifdef HAVE_MQTT
	QMqttClient::ClientError m_error;
#endif
	LiveDataSource * m_source;

private slots:
	void makeChange();

signals:

};
#endif

#endif // MQTTERRORWIDGET_H
