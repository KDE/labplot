/*
	File                 : LiveDataTest.cpp
	Project              : LabPlot
	Description          : Tests for reading live data from files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LiveDataTest.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"

// ##############################################################################
// #################  reading the whole file on changes #########################
// ##############################################################################

/*!
 * comma separated ASCII data, read whole file on changes, without header
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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled));

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
 * comma separated ASCII data, read whole file on changes, with header
 */
void LiveDataTest::testReadWholeFile01() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("x,y\n1,2\n");
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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(true);
	filter->setHeaderLine(1);
	dataSource.setFilter(filter);

	// read the data and perform checks
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 1);

	QCOMPARE(dataSource.column(0)->name(), QStringLiteral("x"));
	QCOMPARE(dataSource.column(1)->name(), QStringLiteral("y"));

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled));

	// checks
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->name(), QStringLiteral("x"));
	QCOMPARE(dataSource.column(1)->name(), QStringLiteral("y"));

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);
}

// ##############################################################################
// ##########################  helper functions #################################
// ##############################################################################

void LiveDataTest::waitForSignal(QObject* sender, const char* signal) {
	QTimer timeout(this);
	timeout.setSingleShot(true);

	QEventLoop loop;
	connect(sender, signal, &loop, SLOT(quit()));
	connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
	timeout.start(1000);
	loop.exec();
}

QTEST_MAIN(LiveDataTest)
