/*
	File                 : LiveDataWriteFileTest.cpp
	Project              : LabPlot
	Description          : Tests for Livedata with files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LiveDataWriteFileTest.h"

#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AsciiFilter.h"

#include <QProcess>

void LiveDataWriteFileTest::init() {
	m_process.setProgram(QStringLiteral(LIVEDATAWRITEFILE_EXEC));
	m_process.start();
	QVERIFY(m_process.waitForStarted());
}

void LiveDataWriteFileTest::cleanup() {
	m_process.terminate();
	QVERIFY(m_process.waitForFinished());
}

/*!
 * \brief LiveDataWriteFileTest::testRefresh
 * Testing if the refresh is comming at the correct rate
 */
void LiveDataWriteFileTest::testRefresh() {
	auto* dataSource = new LiveDataSource(i18n("Live data source%1", 1), false);
	dataSource->setFileType(AbstractFileFilter::FileType::Ascii);

	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(false);

	filter->setCreateTimestampEnabled(false);
	// filter->setStartRow(1);
	filter->setEndRow(-1);
	filter->setStartColumn(0);
	filter->setEndColumn(-1);
	filter->setHeaderEnabled(true); // header in line 1
	filter->setHeaderLine(0);
	filter->setSeparatingCharacter(QStringLiteral(","));

	dataSource->setFilter(filter);
	// dataSource->setUpdateInterval(1000); // Not relevant
	dataSource->setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource->setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource->setFileName(QStringLiteral(EXPORT_FILE));
	dataSource->setUpdateType(LiveDataSource::UpdateType::NewData);

	qint64 lastUpdate = 0;
	int counter = 0;
	bool timeoutReadOnUpdateCalled = false;
	connect(dataSource, &LiveDataSource::readOnUpdateCalled, [&lastUpdate, &counter, &timeoutReadOnUpdateCalled](bool paused) {
		const qint64 t = QDateTime::currentDateTime().toMSecsSinceEpoch();
		counter++;

		QCOMPARE(paused, false);

		if (lastUpdate > 0) {
			const auto diff = lastUpdate - t;
			const auto timeout = abs(diff - SLEEP_TIME_MS) < static_cast<double>(SLEEP_TIME_MS) / 10; // maximum 10% of delay
			if (timeout)
				timeoutReadOnUpdateCalled = true;
		}
		lastUpdate = t;
	});

	const auto startTime = std::chrono::high_resolution_clock::now();
	bool timeout = false;
	const qint64 timeoutTime = 100 * SLEEP_TIME_MS * 1.1; // 1.1; // 10% tolearance
	while (counter < 100 && !timeout) {
		const auto currTime = std::chrono::high_resolution_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime).count();
		if (diff > timeoutTime) {
			timeout = true;
		}
		QApplication::processEvents();
	}
	QCOMPARE(timeoutReadOnUpdateCalled, false);
	QCOMPARE(timeout, false);
}

// setHeaderLine is set greater than 0
// It should not crash
// void LiveDataWriteFileTest::testRefreshHeaderLine() {
//	auto* dataSource = new LiveDataSource(i18n("Live data source%1", 1), false);
//	dataSource->setFileType(AbstractFileFilter::FileType::Ascii);

//	AsciiFilter* filter = new AsciiFilter();
//	filter->setAutoModeEnabled(false);

//	filter->setCreateTimestampEnabled(false);
//	// filter->setStartRow(1);
//	filter->setEndRow(-1);
//	filter->setStartColumn(0);
//	filter->setEndColumn(-1);
//	filter->setHeaderEnabled(true); // header in line 1
//    filter->setHeaderLine(1); // Header line set to a value greater than 1!
//	filter->setSeparatingCharacter(QStringLiteral(","));

//	dataSource->setFilter(filter);
//	// dataSource->setUpdateInterval(1000); // Not relevant
//	dataSource->setSourceType(LiveDataSource::SourceType::FileOrPipe);
//	dataSource->setReadingType(LiveDataSource::ReadingType::WholeFile);
//	dataSource->setFileName(QStringLiteral(EXPORT_FILE));
//	dataSource->setUpdateType(LiveDataSource::UpdateType::NewData);

//	qint64 lastUpdate = 0;
//	int counter = 0;
//	bool timeoutReadOnUpdateCalled = false;
//	connect(dataSource, &LiveDataSource::readOnUpdateCalled, [&lastUpdate, &counter, &timeoutReadOnUpdateCalled](bool paused) {
//		const qint64 t = QDateTime::currentDateTime().toMSecsSinceEpoch();
//		counter++;

//		QCOMPARE(paused, false);

//		if (lastUpdate > 0) {
//			const auto diff = lastUpdate - t;
//			const auto timeout = abs(diff - SLEEP_TIME_MS) < static_cast<double>(SLEEP_TIME_MS) / 10; // maximum 10% of delay
//			if (timeout)
//				timeoutReadOnUpdateCalled = true;
//		}
//		lastUpdate = t;
//	});

//	const auto startTime = std::chrono::high_resolution_clock::now();
//	bool timeout = false;
//	const qint64 timeoutTime = 100 * SLEEP_TIME_MS * 1.1; // 1.1; // 10% tolearance
//	while (counter < 100 && !timeout) {
//		const auto currTime = std::chrono::high_resolution_clock::now();
//		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime).count();
//		if (diff > timeoutTime) {
//			timeout = true;
//		}
//		QApplication::processEvents();
//	}
//	QCOMPARE(timeoutReadOnUpdateCalled, false);
//	QCOMPARE(timeout, false);
//}
QTEST_MAIN(LiveDataWriteFileTest)
