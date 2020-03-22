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
#include "kdefrontend/dockwidgets/LiveDataDock.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QEventLoop>
#include <QTreeWidgetItem>

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
			QStringList topics = line.split(' ', QString::SkipEmptyParts);
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

//##############################################################################
//############################  check common topics  ###########################
//##############################################################################
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

//##############################################################################
//#################  test handling of data received by messages  ###############
//##############################################################################
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
	mqttClient->setMQTTClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	bool wait = QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);
	QCOMPARE(wait, true);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "integer_message_1.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			QString message = in.readAll();
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::MQTTTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			if (testTopic) {
				Column* value = testTopic->column(testTopic->columnCount() - 1);
				QCOMPARE(value->columnMode(), Column::ColumnMode::Integer);
				QCOMPARE(value->rowCount(), 3);
				QCOMPARE(value->valueAt(0), 1);
				QCOMPARE(value->valueAt(1), 2);
				QCOMPARE(value->valueAt(2), 3);

				const QString fileName2 = m_dataDir + "integer_message_2.txt";
				QFile file2(fileName2);

				if(file2.open(QIODevice::ReadOnly)) {
					QTextStream in2(&file2);
					QString message = in2.readAll();
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
	mqttClient->setMQTTClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	bool wait = QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);
	QCOMPARE(wait, true);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "numeric_message_1.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			QString message = in.readAll();
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::MQTTTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			if (testTopic) {
				Column* value = testTopic->column(testTopic->columnCount() - 1);
				QCOMPARE(value->columnMode(), Column::ColumnMode::Numeric);
				QCOMPARE(value->rowCount(), 3);
				QCOMPARE(value->valueAt(0), 1.5);
				QCOMPARE(value->valueAt(1), 2.7);
				QCOMPARE(value->valueAt(2), 3.9);

				const QString fileName2 = m_dataDir + "numeric_message_2.txt";
				QFile file2(fileName2);

				if(file2.open(QIODevice::ReadOnly)) {
					QTextStream in2(&file2);
					QString message = in2.readAll();
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
	mqttClient->setMQTTClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttClient->read();
	mqttClient->ready();

	QMqttClient* client = new QMqttClient();
	client->setHostname("broker.hivemq.com");
	client->setPort(1883);
	client->connectToHost();

	bool wait = QTest::qWaitFor([&]() {
		return (client->state() == QMqttClient::Connected);
	}, 5000);
	QCOMPARE(wait, true);

	QMqttSubscription* subscription = client->subscribe(topicFilter, 0);
	if(subscription) {
		const QString fileName = m_dataDir + "text_message.txt";
		QFile file(fileName);

		if(file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			QString message = in.readAll();
			client->publish(topicFilter.filter(), message.toUtf8(), 0);
		}
		file.close();


		QTimer timer;
		timer.setSingleShot(true);
		QEventLoop* loop = new QEventLoop();
		connect(mqttClient, &MQTTClient::MQTTTopicsChanged, loop, &QEventLoop::quit);
		connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
		timer.start(5000);
		loop->exec();

		const MQTTTopic* testTopic = nullptr;

		if(timer.isActive()) {
			QVector<const MQTTTopic*> topic = mqttClient->children <const MQTTTopic>(AbstractAspect::Recursive);
			for(int i = 0; i < topic.size(); ++i) {
				if (topic[i]->topicName() == "labplot/mqttUnitTest") {
					testTopic = topic[i];
					break;
				}
			}

			if (testTopic) {
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
}

//##############################################################################
//#####################  test subscribing and unsubscribing  ###################
//##############################################################################
void MQTTUnitTest::testSubscriptions() {
	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(true);

	Project* project = new Project();

	MQTTClient* mqttClient = new MQTTClient("test");
	project->addChild(mqttClient);
	mqttClient->setFilter(filter);
	mqttClient->setReadingType(MQTTClient::TillEnd);
	mqttClient->setKeepNValues(0);
	mqttClient->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttClient->setMQTTClientHostPort("broker.hivemq.com", 1883);
	mqttClient->setMQTTUseAuthentication(false);
	mqttClient->setMQTTUseID(false);
	mqttClient->setMQTTWillUse(false);
	QMqttTopicFilter topicFilter {"labplot/mqttUnitTest"};
	mqttClient->addInitialMQTTSubscriptions(topicFilter, 0);

	LiveDataDock* liveDock = new LiveDataDock();
	liveDock->setMQTTClient(mqttClient);

	mqttClient->read();
	mqttClient->ready();

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop* loop = new QEventLoop();
	connect(mqttClient, &MQTTClient::MQTTSubscribed, loop, &QEventLoop::quit);
	connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
	timer.start(5000);
	loop->exec();

	if(timer.isActive()) {
		delete loop;
		QMqttClient* client = new QMqttClient();
		client->setHostname("broker.hivemq.com");
		client->setPort(1883);
		client->connectToHost();

		bool wait = QTest::qWaitFor([&]() {
			return (client->state() == QMqttClient::Connected);
		}, 3000);
		QCOMPARE(wait, true);

		QString fileName = m_dataDir + "subscribe_1.txt";
		QFile* file = new QFile(fileName);

		QTest::qWait(1000);

		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);

			while(!in.atEnd()) {
				QString line = in.readLine();
				QMqttTopicFilter filter{line};
				client->publish(filter.filter(), QString("test").toUtf8());

				QTimer timer2;
				timer2.setSingleShot(true);
				loop = new QEventLoop();
				connect( (&timer2), &QTimer::timeout, loop, &QEventLoop::quit);
				connect(liveDock, &LiveDataDock::newTopic, this, [line, loop](const QString& topic) {
					if(topic == line) {
						loop->quit();
					}
				});
				timer2.start(5000);
				loop->exec();

				disconnect(liveDock, &LiveDataDock::newTopic, this, nullptr);
			}
		}

		liveDock->testUnsubscribe("labplot/mqttUnitTest");

		file->close();
		delete file;

		fileName = m_dataDir + "subscribe_2.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			while(!in.atEnd()) {
				QString topic = in.readLine();
				bool found = liveDock->testSubscribe(topic);
				QCOMPARE(found, true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "subscribe_2_result.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			int count = in.readLine().simplified().toInt();
			QCOMPARE(mqttClient->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				QVector<QString> subscriptions = mqttClient->MQTTSubscriptions();
				QCOMPARE(subscriptions.contains(topic), true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "unsubscribe_1.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			while(!in.atEnd()) {
				QString topic = in.readLine();
				bool found = liveDock->testUnsubscribe(topic);
				QCOMPARE(found, true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "unsubscribe_1_result.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			int count = in.readLine().simplified().toInt();
			QCOMPARE(mqttClient->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				QVector<QString> subscriptions = mqttClient->MQTTSubscriptions();
				QCOMPARE(subscriptions.contains(topic), true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "subscribe_3.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			while(!in.atEnd()) {
				QString topic = in.readLine();
				QVector<QString> subscriptions = mqttClient->MQTTSubscriptions();
				bool found = liveDock->testSubscribe(topic);
				QCOMPARE(found, true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "subscribe_3_result.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			int count = in.readLine().simplified().toInt();
			QVector<QString> sub = mqttClient->MQTTSubscriptions();
			QCOMPARE(mqttClient->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				QVector<QString> subscriptions = mqttClient->MQTTSubscriptions();
				QCOMPARE(subscriptions.contains(topic), true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "unsubscribe_2.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			while(!in.atEnd()) {
				QString topic = in.readLine();
				bool found = liveDock->testUnsubscribe(topic);
				QCOMPARE(found, true);
			}
		}
		file->close();
		delete file;

		fileName = m_dataDir + "unsubscribe_2_result.txt";
		file = new QFile(fileName);
		if(file->open(QIODevice::ReadOnly)) {
			QTextStream in(file);
			int count = in.readLine().simplified().toInt();
			QCOMPARE(mqttClient->MQTTSubscriptions().size(), count);

			QVector<QString> subscriptions = mqttClient->MQTTSubscriptions();
			while(!in.atEnd()) {
				QString topic = in.readLine();
				QCOMPARE(subscriptions.contains(topic), true);
			}
		}
		file->close();
		delete file;
	}
}

QTEST_MAIN(MQTTUnitTest)

#endif //HAVE_MQTT
