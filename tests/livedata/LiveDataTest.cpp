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

#include <QEventLoop>
#include <QTimer>

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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

	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
	filter->setCreateIndexEnabled(true);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 2);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("5,6\n7,8\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 3);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 1);
	QCOMPARE(dataSource.column(2)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 2);
	QCOMPARE(dataSource.column(1)->integerAt(1), 3);
	QCOMPARE(dataSource.column(2)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 3);
	QCOMPARE(dataSource.column(1)->integerAt(2), 5);
	QCOMPARE(dataSource.column(2)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 4);
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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
	filter->setCreateTimestampEnabled(true);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
	filter->setCreateIndexEnabled(true);
	filter->setCreateTimestampEnabled(true);
	dataSource.setFilter(filter);

	// read the data and perform checks, after the initial read all data is read
	dataSource.read();

	QCOMPARE(dataSource.columnCount(), 4);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(0), 1);
	QCOMPARE(dataSource.column(3)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 2);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

	// all new data (2 new lines) was added, check
	QCOMPARE(dataSource.columnCount(), 4);
	QCOMPARE(dataSource.rowCount(), 4);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::DateTime);
	QCOMPARE(dataSource.column(2)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(3)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->dateTimeAt(0).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(0), 1);
	QCOMPARE(dataSource.column(3)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 2);
	QCOMPARE(dataSource.column(1)->dateTimeAt(1).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(1), 3);
	QCOMPARE(dataSource.column(3)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 3);
	QCOMPARE(dataSource.column(1)->dateTimeAt(2).isValid(), true);
	QCOMPARE(dataSource.column(2)->integerAt(2), 5);
	QCOMPARE(dataSource.column(3)->integerAt(2), 6);

	QCOMPARE(dataSource.column(0)->integerAt(3), 4);
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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 3);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 1);
	QCOMPARE(dataSource.column(1)->integerAt(0), 2);

	QCOMPARE(dataSource.column(0)->integerAt(1), 3);
	QCOMPARE(dataSource.column(1)->integerAt(1), 4);

	QCOMPARE(dataSource.column(0)->integerAt(2), 7);
	QCOMPARE(dataSource.column(1)->integerAt(2), 8);
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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

	// the first line of the new data (sample size = 1) was added, check
	QCOMPARE(dataSource.columnCount(), 2);
	QCOMPARE(dataSource.rowCount(), 2);

	QCOMPARE(dataSource.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(dataSource.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(dataSource.column(0)->integerAt(0), 3);
	QCOMPARE(dataSource.column(1)->integerAt(0), 4);

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	filter->setSeparatingCharacter(QStringLiteral(","));
	filter->setHeaderEnabled(false);
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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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

/*!
 * comma separated ASCII data, read whole file on changes, with header in the first line, append new data
 */
void LiveDataTest::testReadWholeFile02() {
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

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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

/*!
 * comma separated ASCII data, read whole file on changes, with header in the second line, append new data
 */
void LiveDataTest::testReadWholeFile03() {
	// create a temp file and write some data into it
	QTemporaryFile tempFile;
	if (!tempFile.open())
		QFAIL("failed to create the temp file for writing");

	QFile file(tempFile.fileName());
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
		QFAIL("failed to open the temp file for writing");

	file.write("ignore\nx,y\n1,2\n");
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
	filter->setHeaderLine(2);
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

// currently fails on Windows (waitForSignal()?)
#ifdef HAVE_WINDOWS
	return;
#endif
	// write out more data to the file
	file.write("3,4\n");
	file.close();
	waitForSignal(&dataSource, SIGNAL(readOnUpdateCalled()));

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
	QTimer timer(this);
	timer.setSingleShot(true);

	QEventLoop loop;
	QTimer::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	QObject::connect(sender, signal, &loop, SLOT(quit()));
	timer.start(1000);
	loop.exec();
}

QTEST_MAIN(LiveDataTest)
