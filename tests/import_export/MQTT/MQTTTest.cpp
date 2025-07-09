/*
	File                 : MQTTTest.cpp
	Project              : LabPlot
	Description          : Tests for MQTT related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MQTTTest.h"

#ifdef HAVE_MQTT
#include "backend/core/Project.h"
#include "backend/datasources/MQTTClient.h"
#include "backend/datasources/MQTTSubscription.h"
#include "backend/datasources/MQTTTopic.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "frontend/dockwidgets/LiveDataDock.h"

#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QVector>

#include <iostream>

namespace {
const QLatin1String mqttHostName(HOSTNAME);
const int mqttPort = PORT;
constexpr double timeout_ms = 10000.0;
}

// This is not yet required. Can be used to start an external process which publishes mqtt messages
// void MQTTTest::init() {
// 	const auto executable = QStringLiteral(EXEC);
// 	m_process.setProgram(executable);
// 	m_process.start();
// 	QVERIFY(m_process.waitForStarted());
// 	// const auto e1 = m_process.errorString();
// }

// void MQTTTest::cleanup() {
// 	m_process.terminate();
// 	QVERIFY(m_process.waitForFinished());
// }

// For manual validation of the publish use a client like MQTTX (https://mqttx.app/)
// Connect using HOSTNAME and PORT and listen on the "labplot/mqttUnitTest" topic

void MQTTTest::initTestCase() {
	CommonTest::initTestCase();

	const QString currentDir = QLatin1String(__FILE__);
	m_dataDir = currentDir.left(currentDir.lastIndexOf(QDir::separator())) + QDir::separator() + QLatin1String("data") + QDir::separator();
}

// ##############################################################################
// ###################  check superior and inferior relations  ##################
// ##############################################################################
void MQTTTest::testContainFalse() {
	auto* client = new MQTTClient(QStringLiteral("test"));
	const QString fileName = m_dataDir + QStringLiteral("contain_false.txt");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (!in.atEnd()) {
			QString line = in.readLine();
			auto topics = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
			QCOMPARE(client->checkTopicContains(topics[0], topics[1]), false);
		}

		delete client;
		file.close();
	}
}

void MQTTTest::testContainTrue() {
	auto* client = new MQTTClient(QStringLiteral("test"));
	const QString fileName = m_dataDir + QStringLiteral("contain_true.txt");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (!in.atEnd()) {
			QString line = in.readLine();
			auto topics = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
			QCOMPARE(client->checkTopicContains(topics[0], topics[1]), true);
		}

		delete client;
		file.close();
	}
}

// ##############################################################################
// ############################  check common topics  ###########################
// ##############################################################################
void MQTTTest::testCommonTrue() {
	auto* client = new MQTTClient(QStringLiteral("test"));
	const QString fileName = m_dataDir + QStringLiteral("common_true.txt");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (!in.atEnd()) {
			QString line = in.readLine();
			auto topics = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
			QCOMPARE(client->checkCommonLevel(topics[0], topics[1]), topics[2]);
		}

		delete client;
		file.close();
	}
}

void MQTTTest::testCommonFalse() {
	auto* client = new MQTTClient(QStringLiteral("test"));
	const QString fileName = m_dataDir + QStringLiteral("common_false.txt");
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);

		while (!in.atEnd()) {
			QString line = in.readLine();
			auto topics = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
			QCOMPARE(client->checkCommonLevel(topics[0], topics[1]), QString());
		}

		delete client;
		file.close();
	}
}

// ##############################################################################
// #################  test handling of data received by messages  ###############
// ##############################################################################
void MQTTTest::testIntegerMessage() {
	auto* filter = new AsciiFilter();

	auto properties = filter->properties();
	properties.automaticSeparatorDetection = false;
	properties.headerEnabled = false;
	properties.columnModesString = QStringLiteral("Int");
	properties.intAsDouble = false;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success); // Livedata must be initialized!

	auto* project = new Project();

	auto* mqttReceiver = new MQTTClient(QStringLiteral("test"));
	project->addChild(mqttReceiver);
	mqttReceiver->setFilter(filter);
	mqttReceiver->setReadingType(MQTTClient::ReadingType::TillEnd);
	mqttReceiver->setKeepNValues(100); // At least 8!
	mqttReceiver->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttReceiver->setMQTTClientHostPort(mqttHostName, mqttPort);
	mqttReceiver->setMQTTUseAuthentication(false);
	mqttReceiver->setMQTTUseID(false);
	QMqttTopicFilter topicFilter{QStringLiteral("labplot/mqttUnitTest")};
	mqttReceiver->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttReceiver->read();
	mqttReceiver->ready();

	auto* publisherClient = new QMqttClient();
	publisherClient->setHostname(mqttHostName);
	publisherClient->setPort(mqttPort);
	publisherClient->connectToHost();

	QCOMPARE(waitForSignal(publisherClient, &QMqttClient::connected, timeout_ms), true);

	auto* subscription = publisherClient->subscribe(topicFilter, 0);
	if (subscription) {
		{
			const QStringList data = {
				QStringLiteral("1"),
				QStringLiteral("2"),
				QStringLiteral("3"),
			};
			QString savePath;
			SAVE_FILE("testfile", data);
			QFile file(savePath);

			if (file.open(QIODevice::ReadOnly)) {
				QTextStream in(&file);
				QString message = in.readAll();
				publisherClient->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file.close();
		}

		QCOMPARE(waitForSignal(mqttReceiver, &MQTTClient::messagedReceived, timeout_ms), true);

		const MQTTTopic* testTopic = nullptr;
		auto topic = mqttReceiver->children<const MQTTTopic>(AbstractAspect::ChildIndexFlag::Recursive);
		for (const auto& top : topic) {
			if (top->topicName() == QLatin1String("labplot/mqttUnitTest")) {
				testTopic = top;
				break;
			}
		}
		QVERIFY(testTopic);

		Column* value = testTopic->column(testTopic->columnCount() - 1);
		QCOMPARE(value->columnMode(), Column::ColumnMode::Integer);
		QCOMPARE(value->rowCount(), 3);
		QCOMPARE(value->integerAt(0), 1);
		QCOMPARE(value->integerAt(1), 2);
		QCOMPARE(value->integerAt(2), 3);

		{
			const QStringList data = {
				QStringLiteral("5"),
				QStringLiteral("6"),
				QStringLiteral("7"),
				QStringLiteral("8"),
			};
			QString savePath;
			SAVE_FILE("testfile2", data);
			QFile file2(savePath);

			if (file2.open(QIODevice::ReadOnly)) {
				QTextStream in2(&file2);
				QString message = in2.readAll();
				publisherClient->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file2.close();
		}

		QCOMPARE(waitForSignal(mqttReceiver, &MQTTClient::messagedReceived, timeout_ms), true);

		QCOMPARE(value->rowCount(), 7);
		QCOMPARE(value->integerAt(0), 1);
		QCOMPARE(value->integerAt(1), 2);
		QCOMPARE(value->integerAt(2), 3);
		QCOMPARE(value->integerAt(3), 5);
		QCOMPARE(value->integerAt(4), 6);
		QCOMPARE(value->integerAt(5), 7);
		QCOMPARE(value->integerAt(6), 8);
	}
}

void MQTTTest::testNumericMessage() {
	auto* filter = new AsciiFilter();

	auto properties = filter->properties();
	properties.automaticSeparatorDetection = false;
	properties.headerEnabled = false;
	properties.columnModesString = QStringLiteral("Double");
	properties.intAsDouble = false;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success); // Livedata must be initialized!

	Project* project = new Project();

	auto* mqttReceiver = new MQTTClient(QStringLiteral("test"));
	project->addChild(mqttReceiver);
	mqttReceiver->setFilter(filter);
	mqttReceiver->setReadingType(MQTTClient::ReadingType::TillEnd);
	mqttReceiver->setKeepNValues(100); // At least 8!
	mqttReceiver->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttReceiver->setMQTTClientHostPort(mqttHostName, mqttPort);
	mqttReceiver->setMQTTUseAuthentication(false);
	mqttReceiver->setMQTTUseID(false);
	QMqttTopicFilter topicFilter{QStringLiteral("labplot/mqttUnitTest")};
	mqttReceiver->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttReceiver->read();
	mqttReceiver->ready();

	auto* publisherClient = new QMqttClient();
	publisherClient->setHostname(mqttHostName);
	publisherClient->setPort(mqttPort);
	publisherClient->connectToHost();

	QCOMPARE(waitForSignal(publisherClient, &QMqttClient::connected, timeout_ms), true);

	auto* subscription = publisherClient->subscribe(topicFilter, 0);
	if (subscription) {
		{
			const QStringList data = {
				QStringLiteral("1.5"),
				QStringLiteral(""),
				QStringLiteral(""),
				QStringLiteral("2.7"),
				QStringLiteral(""),
				QStringLiteral(""),
				QStringLiteral("3.9"),
			};
			QString savePath;
			SAVE_FILE("testfile", data);
			QFile file(savePath);

			if (file.open(QIODevice::ReadOnly)) {
				QTextStream in(&file);
				QString message = in.readAll();
				publisherClient->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file.close();
		}

		QCOMPARE(waitForSignal(mqttReceiver, &MQTTClient::messagedReceived, timeout_ms), true);

		const MQTTTopic* testTopic = nullptr;
		auto topic = mqttReceiver->children<const MQTTTopic>(AbstractAspect::ChildIndexFlag::Recursive);
		for (const auto& top : topic) {
			if (top->topicName() == QLatin1String("labplot/mqttUnitTest")) {
				testTopic = top;
				break;
			}
		}
		QVERIFY(testTopic);

		Column* value = testTopic->column(testTopic->columnCount() - 1);
		QCOMPARE(value->columnMode(), Column::ColumnMode::Double);
		QCOMPARE(value->rowCount(), 3);
		QCOMPARE(value->valueAt(0), 1.5);
		QCOMPARE(value->valueAt(1), 2.7);
		QCOMPARE(value->valueAt(2), 3.9);

		{
			const QStringList data = {
				QStringLiteral("6"),
				QStringLiteral(""), // Invalid
				QStringLiteral(""), // Invalid
				QStringLiteral("house     ball"), // Invalid
				QStringLiteral(""), // Invalid
				QStringLiteral(""), // Invalid
				QStringLiteral("car"), // Invalid
				QStringLiteral("0.0098"),
				QStringLiteral("1.0"),
				QStringLiteral("12.42"),
			};
			QString savePath;
			SAVE_FILE("testfile2", data);
			QFile file2(savePath);

			if (file2.open(QIODevice::ReadOnly)) {
				QTextStream in2(&file2);
				QString message = in2.readAll();
				publisherClient->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file2.close();
		}

		QCOMPARE(waitForSignal(mqttReceiver, &MQTTClient::messagedReceived, timeout_ms), true);

		QCOMPARE(value->rowCount(), 9);
		QCOMPARE(value->valueAt(0), 1.5);
		QCOMPARE(value->valueAt(1), 2.7);
		QCOMPARE(value->valueAt(2), 3.9);
		QCOMPARE(value->valueAt(3), 6.);
		QCOMPARE((bool)std::isnan(value->valueAt(4)), true);
		QCOMPARE((bool)std::isnan(value->valueAt(5)), true);
		QCOMPARE(value->valueAt(6), 0.0098);
		QCOMPARE(value->valueAt(7), 1.0);
		QCOMPARE(value->valueAt(8), 12.42);
	}
}

void MQTTTest::testTextMessage() {
	auto* filter = new AsciiFilter();

	auto properties = filter->properties();
	properties.automaticSeparatorDetection = false;
	properties.headerEnabled = false;
	properties.columnModesString = QStringLiteral("Text");
	properties.intAsDouble = false;
	properties.commentCharacter = QStringLiteral("#");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success); // Livedata must be initialized!

	Project* project = new Project();

	auto* mqttReceiver = new MQTTClient(QStringLiteral("test"));
	project->addChild(mqttReceiver);
	mqttReceiver->setFilter(filter);
	mqttReceiver->setReadingType(MQTTClient::ReadingType::TillEnd);
	mqttReceiver->setKeepNValues(100); // At least 8!
	mqttReceiver->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttReceiver->setMQTTClientHostPort(mqttHostName, mqttPort);
	mqttReceiver->setMQTTUseAuthentication(false);
	mqttReceiver->setMQTTUseID(false);
	QMqttTopicFilter topicFilter{QStringLiteral("labplot/mqttUnitTest")};
	mqttReceiver->addInitialMQTTSubscriptions(topicFilter, 0);
	mqttReceiver->read();
	mqttReceiver->ready();

	auto* publisherClient = new QMqttClient();
	publisherClient->setHostname(mqttHostName);
	publisherClient->setPort(mqttPort);
	publisherClient->connectToHost();

	QCOMPARE(waitForSignal(publisherClient, &QMqttClient::connected, timeout_ms), true);

	auto* subscription = publisherClient->subscribe(topicFilter, 0);
	if (subscription) {
		{
			const QStringList data = {
				QStringLiteral("ball"),
				QStringLiteral("cat"),
				QStringLiteral("#comment"),
				QStringLiteral("dog"),
				QStringLiteral("#comment something"),
				QStringLiteral("#comment something new"),
				QStringLiteral("house"),
				QStringLiteral("Barcelona"),
			};
			QString savePath;
			SAVE_FILE("testfile", data);
			QFile file(savePath);

			if (file.open(QIODevice::ReadOnly)) {
				QTextStream in(&file);
				QString message = in.readAll();
				publisherClient->publish(topicFilter.filter(), message.toUtf8(), 0);
			}
			file.close();
		}

		QCOMPARE(waitForSignal(mqttReceiver, &MQTTClient::messagedReceived, timeout_ms), true);

		const MQTTTopic* testTopic = nullptr;
		auto topic = mqttReceiver->children<const MQTTTopic>(AbstractAspect::ChildIndexFlag::Recursive);
		for (const auto& top : topic) {
			if (top->topicName() == QLatin1String("labplot/mqttUnitTest")) {
				testTopic = top;
				break;
			}
		}
		QVERIFY(testTopic);

		Column* value = testTopic->column(testTopic->columnCount() - 1);
		QCOMPARE(value->columnMode(), Column::ColumnMode::Text);
		QCOMPARE(value->rowCount(), 5);
		QCOMPARE(value->textAt(0), QStringLiteral("ball"));
		QCOMPARE(value->textAt(1), QStringLiteral("cat"));
		QCOMPARE(value->textAt(2), QStringLiteral("dog"));
		QCOMPARE(value->textAt(3), QStringLiteral("house"));
		QCOMPARE(value->textAt(4), QStringLiteral("Barcelona"));
	}
}

// ##############################################################################
// #####################  test subscribing and unsubscribing  ###################
// ##############################################################################
/*void MQTTTest::testSubscriptions() {
	auto* filter = new AsciiFilter();
	filter->setAutoModeEnabled(true);

	auto* project = new Project();

	auto* mqttReceiver = new MQTTClient(QStringLiteral("test"));
	project->addChild(mqttReceiver);
	mqttReceiver->setFilter(filter);
	mqttReceiver->setReadingType(MQTTClient::ReadingType::TillEnd);
	mqttReceiver->setKeepNValues(0);
	mqttReceiver->setUpdateType(MQTTClient::UpdateType::NewData);
	mqttReceiver->setMQTTClientHostPort(mqttHostName, mqttPort);
	mqttReceiver->setMQTTUseAuthentication(false);
	mqttReceiver->setMQTTUseID(false);
	mqttReceiver->setMQTTWillUse(false);
	QMqttTopicFilter topicFilter {QStringLiteral("labplot/mqttUnitTest")};
	mqttReceiver->addInitialMQTTSubscriptions(topicFilter, 0);

	auto* liveDock = new LiveDataDock();
	liveDock->setMQTTClient(mqttReceiver);

	mqttReceiver->read();
	mqttReceiver->ready();

	QTimer timer;
	timer.setSingleShot(true);
	auto* loop = new QEventLoop();
	connect(mqttReceiver, &MQTTClient::MQTTSubscribed, loop, &QEventLoop::quit);
	connect( (&timer), &QTimer::timeout, loop, &QEventLoop::quit);
	timer.start(5000);
	loop->exec();

	if(timer.isActive()) {
		delete loop;
		auto* client = new QMqttClient();
		client->setHostname(mqttHostName);
		client->setPort(mqttPort);
		client->connectToHost();

		bool wait = QTest::qWaitFor([&]() {
			return (client->state() == QMqttClient::Connected);
		}, 3000);
		QCOMPARE(wait, true);

		QString fileName = m_dataDir + QStringLiteral("subscribe_1.txt");
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
			QCOMPARE(mqttReceiver->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				auto subscriptions = mqttReceiver->MQTTSubscriptions();
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
			QCOMPARE(mqttReceiver->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				auto subscriptions = mqttReceiver->MQTTSubscriptions();
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
			QCOMPARE(mqttReceiver->MQTTSubscriptions().size(), count);

			while(!in.atEnd()) {
				QString topic = in.readLine();
				auto subscriptions = mqttReceiver->MQTTSubscriptions();
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
			QCOMPARE(mqttReceiver->MQTTSubscriptions().size(), count);

			QVector<QString> subscriptions = mqttReceiver->MQTTSubscriptions();
			while(!in.atEnd()) {
				QString topic = in.readLine();
				QCOMPARE(subscriptions.contains(topic), true);
			}
		}
		file->close();
		delete file;
	}
}*/

QTEST_MAIN(MQTTTest)

#endif // HAVE_MQTT
