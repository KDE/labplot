/***************************************************************************
	File                 : MQTTUnitTest.cpp
	Project              : LabPlot
	Description          : Tests for MQTT related features
	--------------------------------------------------------------------
	Copyright            : (C) 2018 Kovacs Ferencz (kferike98@gmail.com)
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

#include "MQTTUnitTest.h"

#ifdef HAVE_MQTT
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/MQTTTopic.h"
#include "backend/core/Project.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QEventLoop>

void MQTTUnitTest::initTestCase() {
	const QString currentDir = __FILE__;
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();

	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//###################  check superior and inferior relations  ##################
//##############################################################################
void MQTTUnitTest::testContainFalse() {
	MQTTClient* client = new MQTTClient("test");
	const QString fileName = m_dataDir + "contain_false.txt";
	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList topics = line.split(" ", QString::SkipEmptyParts);
			QCOMPARE(client->checkTopicContains(topics[0], topics[1]), false);
		}

		delete client;
		file.close();
	}
}

void MQTTUnitTest::testContainTrue() {
	MQTTClient* client = new MQTTClient("test");
	const QString fileName = m_dataDir + "contain_true.txt";
	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList topics = line.split(" ", QString::SkipEmptyParts);
			QCOMPARE(client->checkTopicContains(topics[0], topics[1]), true);
		}

		delete client;
		file.close();
	}
}

void MQTTUnitTest::testCommonTrue(){
	MQTTClient* client = new MQTTClient("test");
	const QString fileName = m_dataDir + "common_true.txt";
	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList topics = line.split(" ", QString::SkipEmptyParts);
			QCOMPARE(client->checkCommonLevel(topics[0], topics[1]), topics[2]);
		}

		delete client;
		file.close();
	}
}

void MQTTUnitTest::testCommonFalse(){
	MQTTClient* client = new MQTTClient("test");
	const QString fileName = m_dataDir + "common_false.txt";
	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList topics = line.split(" ", QString::SkipEmptyParts);
			QCOMPARE(client->checkCommonLevel(topics[0], topics[1]), "");
		}

		delete client;
		file.close();
	}
}


void MQTTUnitTest::testIntegerMessage() {
	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(true);

	Project* project = new Project();

	MQTTClient* mqttClient = new MQTTClient("test");
	project->addChild(mqttClient);
	mqttClient->setFilter(filter);
	mqttClient->setReadingType(MQTTClient::TillEnd);
	mqttClient->setKeepNValues(0);
	mqttClient->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttClient->setMqttClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMqttSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "integer_message_1.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			qDebug()<<"publish";
			QTextStream in(&file);
			QString message = in.readAll();
			qDebug()<<message;
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::mqttTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			qDebug()<<"search for children";
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			Column* value = testTopic->column(testTopic->columnCount() - 1);
			QCOMPARE(value->columnMode(), Column::ColumnMode::Integer);
			QCOMPARE(value->rowCount(), 3);
			QCOMPARE(value->valueAt(0), 1);
			QCOMPARE(value->valueAt(1), 2);
			QCOMPARE(value->valueAt(2), 3);

			const QString fileName2 = m_dataDir + "integer_message_2.txt";
			QFile file2(fileName2);

			if(file2.open(QIODevice::ReadOnly)) {
				qDebug()<<"publish";
				QTextStream in2(&file2);
				QString message = in2.readAll();
				qDebug()<<message;
				client->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file2.close();

			QTest::qWait(1000);

			QCOMPARE(value->rowCount(), 8);
			QCOMPARE(value->valueAt(3), 6);
			QCOMPARE(value->valueAt(4), 0);
			QCOMPARE(value->valueAt(5), 0);
			QCOMPARE(value->valueAt(6), 0);
			QCOMPARE(value->valueAt(7), 3);
		}
	}
}

void MQTTUnitTest::testNumericMessage() {
	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(true);

	Project* project = new Project();

	MQTTClient* mqttClient = new MQTTClient("test");
	project->addChild(mqttClient);
	mqttClient->setFilter(filter);
	mqttClient->setReadingType(MQTTClient::TillEnd);
	mqttClient->setKeepNValues(0);
	mqttClient->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttClient->setMqttClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMqttSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "numeric_message_1.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			qDebug()<<"publish";
			QTextStream in(&file);
			QString message = in.readAll();
			qDebug()<<message;
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::mqttTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			qDebug()<<"search for children";
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			Column* value = testTopic->column(testTopic->columnCount() - 1);
			QCOMPARE(value->columnMode(), Column::ColumnMode::Numeric);
			QCOMPARE(value->rowCount(), 3);
			QCOMPARE(value->valueAt(0), 1.5);
			QCOMPARE(value->valueAt(1), 2.7);
			QCOMPARE(value->valueAt(2), 3.9);

			const QString fileName2 = m_dataDir + "numeric_message_2.txt";
			QFile file2(fileName2);

			if(file2.open(QIODevice::ReadOnly)) {
				qDebug()<<"publish";
				QTextStream in2(&file2);
				QString message = in2.readAll();
				qDebug()<<message;
				client->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file2.close();

			QTest::qWait(1000);

			QCOMPARE(value->rowCount(), 8);
			QCOMPARE(value->valueAt(3), 6);
			QCOMPARE((bool)std::isnan(value->valueAt(4)), true);
			QCOMPARE((bool)std::isnan(value->valueAt(5)), true);
			QCOMPARE((bool)std::isnan(value->valueAt(6)), true);
			QCOMPARE(value->valueAt(7), 0.0098);
		}
	}
}

void MQTTUnitTest::testTextMessage() {
	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(true);

	Project* project = new Project();

	MQTTClient* mqttClient = new MQTTClient("test");
	project->addChild(mqttClient);
	mqttClient->setFilter(filter);
	mqttClient->setReadingType(MQTTClient::TillEnd);
	mqttClient->setKeepNValues(0);
	mqttClient->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttClient->setMqttClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMqttSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "text_message.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			qDebug()<<"publish";
			QTextStream in(&file);
			QString message = in.readAll();
			qDebug()<<message;
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::mqttTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			qDebug()<<"search for children";
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			Column* value = testTopic->column(testTopic->columnCount() - 1);
			QCOMPARE(value->columnMode(), Column::ColumnMode::Text);
			QCOMPARE(value->rowCount(), 5);
			QCOMPARE(value->textAt(0), "ball");
			QCOMPARE(value->textAt(1), "cat");
			QCOMPARE(value->textAt(2), "dog");
			QCOMPARE(value->textAt(3), "house");
			QCOMPARE(value->textAt(4), "Barcelona");
		}
	}
}

QTEST_MAIN(MQTTUnitTest)

#endif //HAVE_MQTT
