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
#endif

class MqttErrorWidget : public QWidget {
	Q_OBJECT

public:
	MqttErrorWidget(QWidget* parent = 0, int errorType = 0, LiveDataSource * source = 0);
	~MqttErrorWidget();


private:
	Ui::MqttErrorWidget ui;
#ifdef HAVE_MQTT
	QMqttClient::ClientError m_error;
#endif
	LiveDataSource * m_source;

private slots:
	void makeChange();

signals:

};
#endif // MQTTERRORWIDGET_H
