/*
	File                 : SerialPortTest.cpp
	Project              : LabPlot
	Description          : Tests for Serial Port related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SerialPortTest.h"
#include "backend/core/Project.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"

void SerialPortTest::initTestCase() {
	CommonTest::initTestCase();

#ifndef _WIN32
	QProcess p;
	p.start(QStringLiteral("which"), {QStringLiteral("socat")});
	QVERIFY2(p.waitForStarted(), STDSTRING(p.errorString()).data());
	QVERIFY(p.waitForFinished());

	const auto whichResult = UTF8_QSTRING(p.readAllStandardOutput()).trimmed();
	m_socat_command_available = !whichResult.isEmpty();
#else
	m_socat_command_available = false;
#endif

	if (!m_socat_command_available)
		return;

	m_process_socat.setProgram(QStringLiteral("socat"));
	m_process_socat.setArguments({QStringLiteral("-d2"), QStringLiteral("pty,raw,echo=0"), QStringLiteral("pty,raw,echo=0")});
	m_process_socat.start();
	QVERIFY(m_process_socat.waitForStarted()); //, STDSTRING(m_process_socat.errorString()).data());

	QVERIFY(waitForSignal(&m_process_socat, &QProcess::readyReadStandardError, 5000));

	const auto se = UTF8_QSTRING(m_process_socat.readAllStandardError()); // Socat uses stderr for output by default
	DEBUG(Q_FUNC_INFO << ", Output:" << STDSTRING(se));

	QRegularExpression re(QStringLiteral(".*(?<device>/dev/[a-zA-z0-9]*/[0-9]*)$"));

	const auto f = se.split(QStringLiteral("\n"));
	QVERIFY(f.size() >= 2);
	auto m = re.match(f.at(0));
	QVERIFY(m.hasMatch());
	m_senderDevice = m.captured(QStringLiteral("device"));

	m = re.match(f.at(1));
	QVERIFY(m.hasMatch());
	m_receiverDevice = m.captured(QStringLiteral("device"));

	// brackets are required around the command!
	// const auto command = QStringLiteral("(while true; do echo Sending; echo $RANDOM,$RANDOM,$RANDOM.$RANDOM > %1; sleep %2; done)").arg(senderDevice).arg(1);
	// DEBUG("Command: " << STDSTRING(command));

	// connect(&m_process_send,&QProcess::errorOccurred, [this] () {
	// 	QVERIFY2(false, STDSTRING(m_process_send.errorString()).data());
	// });
	// connect(&m_process_send, &QProcess::readyReadStandardError, [this](){
	// 	QVERIFY2(false, STDSTRING(m_process_send.readAllStandardError()).data());
	// });
	// // connect(&m_process_send, &QProcess::finished, [this](){
	// // 	DEBUG("m_process_send: " << STDSTRING(m_process_send.readAllStandardOutput()));
	// // 	DEBUG("m_process_send std error: " << STDSTRING(m_process_send.readAllStandardError()));
	// // 	QVERIFY2(false, STDSTRING(m_process_send.errorString()).data());
	// // });
	// m_process_send.start(QStringLiteral("/bin/bash"), QStringList() << QStringLiteral("-c") << command);
	// QVERIFY2(m_process_send.waitForStarted(), STDSTRING(m_process_send.errorString()).data());
}

void SerialPortTest::cleanupTestCase() {
	m_process_send.kill();
	m_process_socat.kill();
}

void SerialPortTest::testReading() {
	if (!m_socat_command_available)
		QSKIP("No Socat available, so it is not possible to execute this test!", SkipSingle);

	auto* filter = new AsciiFilter();
	auto properties = filter->properties();
	properties.automaticSeparatorDetection = false;
	properties.headerEnabled = false;
	properties.columnModesString = QStringLiteral("Int,Int,Double");
	properties.intAsDouble = false;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success); // Live data must be initialized!

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::SerialPort);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setSerialPort(m_receiverDevice);
	dataSource.setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);
	dataSource.setFilter(filter);

	const auto command_template = QStringLiteral("(echo %1 > %2)");
	connect(&m_process_send, &QProcess::errorOccurred, []() {
		QVERIFY(false); //, STDSTRING(m_process_send.errorString()).data());
	});
	connect(&m_process_send, &QProcess::readyReadStandardError, []() {
		QVERIFY(false); //, STDSTRING(UTF8_QSTRING(m_process_send.readAllStandardError())).data());
	});

	// read the data and perform checks

	auto data = QStringLiteral("'1,2,3.4345\n2,5,-234\n3,490,293.65\n4,23,0.0001\n'");
	m_process_send.start(QStringLiteral("/bin/bash"), QStringList() << QStringLiteral("-c") << command_template.arg(data).arg(m_senderDevice));
	QVERIFY(m_process_send.waitForStarted()); //, STDSTRING(m_process_send.errorString()).data());
	dataSource.read();

	wait(2000);

	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 3); // first message will be skipped, because we don't know if the line is complete

	QCOMPARE(dataSource.column(0)->integerAt(0), 2);
	QCOMPARE(dataSource.column(1)->integerAt(0), 5);
	VALUES_EQUAL(dataSource.column(2)->valueAt(0), -234.);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 490);
	VALUES_EQUAL(dataSource.column(2)->valueAt(1), 293.65);

	QCOMPARE(dataSource.column(0)->integerAt(2), 4);
	QCOMPARE(dataSource.column(1)->integerAt(2), 23);
	VALUES_EQUAL(dataSource.column(2)->valueAt(2), 0.0001);
}

QTEST_MAIN(SerialPortTest)
