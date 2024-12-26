/*
	File                 : LiveDataTest.cpp
	Project              : LabPlot
	Description          : Tests for reading live data from files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LiveDataTest.h"
#include "backend/core/Project.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <QEventLoop>
#include <QTcpServer>
#include <QTimer>
#include <QUdpSocket>

namespace {
constexpr int udpNewDataUpdateTimeMs = PUBLISH_TIME_MS;
constexpr QLatin1String hostname = QLatin1String(HOSTNAME);
} // anonymous namespace

void LiveDataTest::initTestCase() {
	CommonTest::initTestCase();

	// initialize the TCP socket/server
	m_tcpServer = new QTcpServer(this);
	if (!m_tcpServer->listen(QHostAddress(hostname), TCP_PORT))
		QFAIL("Failed to start the TCP server. " /* + QString(m_tcpServer->errorString())*/);

	m_tcpSendTimer = new QTimer(this);
	m_tcpSendTimer->setInterval(PUBLISH_TIME_MS);

	connect(m_tcpServer, &QTcpServer::newConnection, [this]() {
		m_tcpSocket = m_tcpServer->nextPendingConnection();
		m_tcpSendTimer->start(PUBLISH_TIME_MS);
	});

	connect(m_tcpSendTimer, &QTimer::timeout, [this]() {
		if (!m_tcpSocket)
			return;
		if (!m_tcpSocket->isOpen()) {
			delete m_tcpSocket;
			m_tcpSocket = nullptr;
		}
		QByteArray block = QStringLiteral("1,2\n").toLatin1();
		m_tcpSocket->write(block);
	});

	int udpNewDataUpdateTimeMs = PUBLISH_TIME_MS;

	// initialize the UDP socket
	m_udpSocket = new QUdpSocket(this);
	QVERIFY (m_udpSocket->bind(QHostAddress(hostname), 56080)); // This port must be different to the udp port!
	auto* timer = new QTimer(this);
	QCoreApplication::connect(timer, &QTimer::timeout, [this]() {
		this->m_udpSocket->writeDatagram("1,2", QHostAddress(QStringLiteral(HOSTNAME)), UDP_PORT);
	});
	timer->start(udpNewDataUpdateTimeMs);
}

void LiveDataTest::cleanupTestCase() {
}

// ##############################################################################
// Conti. fixed - read fixed number of samples from the beginning of the new data
// ##############################################################################
/*!
 * read from the beginning of the new data, read all samples, keep all data
 */
void LiveDataTest::testReadContinuousFixed00() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);

	// QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	// Watch timer of the LiveDataSource triggered
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 7);
	QCOMPARE(dataSource.column(1)->integerAt(3), 8);
}

/*!
 * read from the beginning of the new data, read 1 sample, keep all data
 */
void LiveDataTest::testReadContinuousFixed01() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(1);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	// Watch timer of the LiveDataSource triggered
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 3);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);
}

/*!
 * read from the beginning of the new data, read 1 sample, keep 2 samples only
 */
void LiveDataTest::testReadContinuousFixed02() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(1);
	dataSource.setKeepNValues(2);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 3);
	QCOMPARE(dataSource.column(1)->integerAt(0), 4);

	QCOMPARE(dataSource.column(0)->integerAt(1), 5);
	QCOMPARE(dataSource.column(1)->integerAt(1), 6);
}

/*!
 * same as testReadContinuousFixed00 but with an additional index column
 */
void LiveDataTest::testReadContinuousFixedWithIndex() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	properties.createIndex = true;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->bigIntAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->bigIntAt(1), 2);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->bigIntAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->bigIntAt(1), 2);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->bigIntAt(2), 3);
	QCOMPARE(dataSource.column(1)->integerAt(2), 5);
	QCOMPARE(dataSource.column(2)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->bigIntAt(3), 4);
	QCOMPARE(dataSource.column(1)->integerAt(3), 7);
	QCOMPARE(dataSource.column(2)->integerAt(3), 8);
}

/*!
 * same as testReadContinuousFixed00 but with an additional timestamp column
 */
void LiveDataTest::testReadContinuousFixedWithTimestamp() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	properties.createTimestamp = true;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(2), 5);
	QCOMPARE(dataSource.column(2)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(1)->integerAt(3), 7);
	QCOMPARE(dataSource.column(2)->integerAt(3), 8);
}

/*!
 * same as testReadContinuousFixed00 but with additional index and timestamp columns
 */
void LiveDataTest::testReadContinuousFixedWithIndexTimestamp() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	properties.createIndex = true;
	properties.createTimestamp = true;
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 4);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->bigIntAt(0), 1);
	QCOMPARE(dataSource.column(1)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(0), 1);
	QCOMPARE(dataSource.column(3)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->bigIntAt(1), 2);
	QCOMPARE(dataSource.column(1)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(1), 3);
	QCOMPARE(dataSource.column(3)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 4);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->bigIntAt(0), 1);
	QCOMPARE(dataSource.column(1)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(0), 1);
	QCOMPARE(dataSource.column(3)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->bigIntAt(1), 2);
	QCOMPARE(dataSource.column(1)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(1), 3);
	QCOMPARE(dataSource.column(3)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->bigIntAt(2), 3);
	QCOMPARE(dataSource.column(1)->dateTimeAt(2).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(2), 5);
	QCOMPARE(dataSource.column(3)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->bigIntAt(3), 4);
	QCOMPARE(dataSource.column(1)->dateTimeAt(3).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(3), 7);
	QCOMPARE(dataSource.column(3)->integerAt(3), 8);
}

// ##############################################################################
// From End - read fixed number of samples from the end of the new data
// #################  reading the whole file on changes #########################
// ##############################################################################
/*!
 * read from the end of the new data, read all data, keep all data
 */
void LiveDataTest::testReadFromEnd00() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::FromEnd);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 7);
	QCOMPARE(dataSource.column(1)->integerAt(3), 8);
}

/*!
 * read from the end of the new data, read 1 sample, keep all data
 */
void LiveDataTest::testReadFromEnd01() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::FromEnd);
	dataSource.setSampleSize(1);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 3);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);

	// read last data
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 7);
	QCOMPARE(dataSource.column(1)->integerAt(3), 8);
}

/*!
 * read from the end of the new data, read 1 sample, keep 2 samples only
 */
void LiveDataTest::testReadFromEnd02() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::FromEnd);
	dataSource.setSampleSize(1);
	dataSource.setKeepNValues(2);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 3);
	QCOMPARE(dataSource.column(1)->integerAt(0), 4);

	QCOMPARE(dataSource.column(0)->integerAt(1), 5);
	QCOMPARE(dataSource.column(1)->integerAt(1), 6);

	dataSource.read();

	QCOMPARE(dataSource.column(0)->integerAt(0), 5);
	QCOMPARE(dataSource.column(1)->integerAt(0), 6);

	QCOMPARE(dataSource.column(0)->integerAt(1), 7);
	QCOMPARE(dataSource.column(1)->integerAt(1), 8);
}

// ##############################################################################
// ############# Till the End - all newly received data is processed ############
// ##############################################################################
/*!
 * read till the end of the new data, read all data, keep all data
 */
void LiveDataTest::testReadTillEnd00() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::TillEnd);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 5);
	QCOMPARE(dataSource.column(1)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 7);
	QCOMPARE(dataSource.column(1)->integerAt(3), 8);
}

/*!
 * read till the end of the new data, read all data, keep 2 samples only
 */
void LiveDataTest::testReadTillEnd01() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n3,4\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::TillEnd);
	dataSource.setKeepNValues(2);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 5);
	QCOMPARE(dataSource.column(1)->integerAt(0), 6);

	QCOMPARE(dataSource.column(0)->integerAt(1), 7);
	QCOMPARE(dataSource.column(1)->integerAt(1), 8);
}

// ##############################################################################
// #################  reading the whole file on changes #########################
// ##############################################################################

/*!
 * comma separated ASCII data, read whole file on changes, without header, append new data
 */
void LiveDataTest::testReadWholeFile00() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);
}

/*!
 * comma separated ASCII data, read whole file on changes, without header, replace the data
 */
void LiveDataTest::testReadWholeFile01() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// close the file, open it again and replace the previous content with the new one
	file.close();
	if (!file.open(QIODevice::ReadWrite))
		QFAIL("failed to open the temp file for writing");
	file.write("3,4\n5,6\n");
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 3);
	QCOMPARE(dataSource.column(1)->integerAt(0), 4);

	QCOMPARE(dataSource.column(0)->integerAt(1), 5);
	QCOMPARE(dataSource.column(1)->integerAt(1), 6);
}

void LiveDataTest::testReadWholeFileSameContentSize() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite))
		QFAIL("failed to open the temp file for writing");

	file.write("1,2\n");
	file.flush();

	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setFileName(file.fileName());
	dataSource.setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource.setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.intAsDouble = false;
	properties.columnNamesRaw = QStringLiteral("x, y");
	properties.columnModesString = QStringLiteral("Int, Int");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// close the file, open it again and replace the previous content with the new one
	file.close();
	if (!file.open(QIODevice::ReadWrite))
		QFAIL("failed to open the temp file for writing");
	file.write("3,4\n"); // The exact same number of lines are used!
	file.close();
	waitForSignal(&dataSource, &LiveDataSource::readOnUpdateCalled);

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 3);
	QCOMPARE(dataSource.column(1)->integerAt(0), 4);
}

void LiveDataTest::testPlotting() {
	Project project;
	auto* worksheet = new Worksheet(QStringLiteral("worksheet"));
	project.addChild(worksheet);

	auto* plot = new CartesianPlot(QStringLiteral("plot"));
	worksheet->addChild(plot);

	auto* curve = new XYCurve(QStringLiteral("curve"));
	plot->addChild(curve);

	auto* dataSource = new LiveDataSource(QStringLiteral("test"), false);
	dataSource->setColumnCount(2);
	dataSource->setRowCount(11);
	project.addChild(dataSource);

	auto* c1 = static_cast<Column*>(dataSource->child<Column>(0));
	QVERIFY(c1 != nullptr);
	QCOMPARE(c1->name(), QLatin1String("1"));
	QVERIFY(c1->columnMode() == AbstractColumn::ColumnMode::Double);
	auto* c2 = static_cast<Column*>(dataSource->child<Column>(1));
	QVERIFY(c2 != nullptr);
	QCOMPARE(c2->name(), QLatin1String("2"));
	QVERIFY(c2->columnMode() == AbstractColumn::ColumnMode::Double);
	for (int i = 0; i < dataSource->rowCount(); i++) {
		c1->setValueAt(i, i);
		c2->setValueAt(i, i);
	}
	curve->setXColumn(c1);
	curve->setYColumn(c2);

	CHECK_RANGE(plot, curve, Dimension::X, 0., 10.);
	CHECK_RANGE(plot, curve, Dimension::Y, 0., 10.);

	QStringList fileContent = {
		QStringLiteral("100,200"),
		QStringLiteral("200,300"),
		QStringLiteral("300,400"),
	};
	QString savePath;
	SAVE_FILE("testfile", fileContent);

	// initialize the live data source
	dataSource->setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource->setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource->setFileName(savePath);
	dataSource->setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource->setUpdateType(LiveDataSource::UpdateType::NewData);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.columnNamesRaw = QStringLiteral("1, 2");
	properties.columnModesString = QStringLiteral("Double, Double");
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	QCOMPARE(filter->initialize(properties), AsciiFilter::Status::Success);
	dataSource->setFilter(filter);

	QCOMPARE(dataSource->column(0)->minimum(), 0.);
	QCOMPARE(dataSource->column(0)->maximum(), 10.);
	QCOMPARE(dataSource->column(1)->minimum(), 0.);
	QCOMPARE(dataSource->column(1)->maximum(), 10.);

	// read the data and perform checks
	dataSource->read(); // first read. The spreadsheet will be prepared for read.

	// checks
	QCOMPARE(dataSource->columnCount(), 2);
	QCOMPARE(dataSource->rowCount(), 3);

	QCOMPARE(dataSource->column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(dataSource->column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(dataSource->column(0)->valueAt(0), 100.);
	QCOMPARE(dataSource->column(1)->valueAt(0), 200.);

	QCOMPARE(dataSource->column(0)->valueAt(1), 200.);
	QCOMPARE(dataSource->column(1)->valueAt(1), 300.);

	QCOMPARE(dataSource->column(0)->valueAt(2), 300.);
	QCOMPARE(dataSource->column(1)->valueAt(2), 400.);

	QCOMPARE(dataSource->column(0)->minimum(), 100.);
	QCOMPARE(dataSource->column(0)->maximum(), 300.);
	QCOMPARE(dataSource->column(1)->minimum(), 200.);
	QCOMPARE(dataSource->column(1)->maximum(), 400.);

	// Check plot ranges again
	CHECK_RANGE(plot, curve, Dimension::X, 100., 300.);
	CHECK_RANGE(plot, curve, Dimension::Y, 200., 400.);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif

	QFile file(savePath);
	if (!file.open(QIODevice::ReadWrite))
		QFAIL("failed to open the temp file for writing");
	file.write("1000,3000\n2000,4000\n3000,8000\n4000,10000\n");
	file.close();
	QCOMPARE(waitForSignal(dataSource, &LiveDataSource::readOnUpdateCalled), false);

	QCOMPARE(dataSource->columnCount(), 2);
	QCOMPARE(dataSource->rowCount(), 4);

	QCOMPARE(dataSource->column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(dataSource->column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(dataSource->column(0)->valueAt(0), 1000.);
	QCOMPARE(dataSource->column(1)->valueAt(0), 3000.);

	QCOMPARE(dataSource->column(0)->valueAt(1), 2000.);
	QCOMPARE(dataSource->column(1)->valueAt(1), 4000.);

	QCOMPARE(dataSource->column(0)->valueAt(2), 3000.);
	QCOMPARE(dataSource->column(1)->valueAt(2), 8000.);

	QCOMPARE(dataSource->column(0)->valueAt(3), 4000.);
	QCOMPARE(dataSource->column(1)->valueAt(3), 10000.);

	QCOMPARE(dataSource->column(0)->minimum(), 1000.);
	QCOMPARE(dataSource->column(0)->maximum(), 4000.);
	QCOMPARE(dataSource->column(1)->minimum(), 3000.);
	QCOMPARE(dataSource->column(1)->maximum(), 10000.);

	CHECK_RANGE(plot, curve, Dimension::X, 1000., 4000.);
	CHECK_RANGE(plot, curve, Dimension::Y, 3000., 10000.);
}

// ##############################################################################
// ##################################  TCP ######################################
// ##############################################################################
namespace {
constexpr int updateIntervalMs = udpNewDataUpdateTimeMs * 100;
}

void LiveDataTest::testTcpReadContinuousFixed00() {
	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::NetworkTCPSocket);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setHost(hostname);
	dataSource.setPort(TCP_PORT);
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::TimeInterval);
	dataSource.setRowCount(0);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	filter->setProperties(properties);
	dataSource.setFilter(filter);

	dataSource.setUpdateInterval(updateIntervalMs); // Start

	// wait until livedata update time is finished and triggers a read
	wait(updateIntervalMs * 3);
	dataSource.pauseReading();

	QCOMPARE(dataSource.columnCount(), 2);
	QVERIFY(dataSource.rowCount() > 0);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(dataSource.column(0)->valueAt(0), 1.);
	QCOMPARE(dataSource.column(1)->valueAt(0), 2.);

	QCOMPARE(dataSource.column(0)->valueAt(dataSource.rowCount() - 1), 1.);
	QCOMPARE(dataSource.column(1)->valueAt(dataSource.rowCount() - 1), 2.);
}

void LiveDataTest::testUdpReadContinuousFixed00() {
	// initialize the live data source
	LiveDataSource dataSource(QStringLiteral("test"), false);
	dataSource.setSourceType(LiveDataSource::SourceType::NetworkUDPSocket);
	dataSource.setFileType(AbstractFileFilter::FileType::Ascii);
	dataSource.setHost(hostname);
	dataSource.setPort(UDP_PORT);
	dataSource.setReadingType(LiveDataSource::ReadingType::ContinuousFixed);
	dataSource.setSampleSize(100); // big number of samples, more then the new data has, meaning we read all new data
	dataSource.setUpdateType(LiveDataSource::UpdateType::TimeInterval);
	dataSource.setRowCount(0);

	// initialize the ASCII filter
	auto* filter = new AsciiFilter();
	auto properties = filter->defaultProperties();
	properties.headerEnabled = false;
	properties.automaticSeparatorDetection = false;
	properties.separator = QStringLiteral(",");
	filter->setProperties(properties);
	dataSource.setFilter(filter);

	dataSource.setUpdateInterval(updateIntervalMs); // Start livedata

	// wait until livedata update time is finished and triggers a read
	wait(updateIntervalMs * 10);
	dataSource.pauseReading();

	QCOMPARE(dataSource.columnCount(), 2);
	QVERIFY(dataSource.rowCount() > 0);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(dataSource.column(0)->valueAt(0), 1.);
	QCOMPARE(dataSource.column(1)->valueAt(0), 2.);

	QCOMPARE(dataSource.column(0)->valueAt(dataSource.rowCount() - 1), 1.);
	QCOMPARE(dataSource.column(1)->valueAt(dataSource.rowCount() - 1), 2.);
}


QTEST_MAIN(LiveDataTest)
