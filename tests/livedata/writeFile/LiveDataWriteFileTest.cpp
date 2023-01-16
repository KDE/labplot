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

/*!
 * \brief LiveDataWriteFileTest::testRefresh
 * Testing if the refresh is comming at the correct rate
 */
void LiveDataWriteFileTest::testRefresh() {
	QProcess process;
	process.setProgram(QStringLiteral(LIVEDATAWRITEFILE_EXEC));
	process.start();
	QVERIFY(process.waitForStarted());

	auto* dataSource = new LiveDataSource(i18n("Live data source%1", 1), false);
	dataSource->setFileType(AbstractFileFilter::FileType::Ascii);

	AsciiFilter* filter = new AsciiFilter();
	filter->setAutoModeEnabled(false);

	filter->setCreateTimestampEnabled(false);
	filter->setStartRow(1);
	filter->setEndRow(-1);
	filter->setStartColumn(0);
	filter->setEndColumn(-1);
	filter->setHeaderEnabled(true); // header in line 1
	filter->setHeaderLine(1);
	filter->setSeparatingCharacter(QStringLiteral(","));

	dataSource->setFilter(filter);
	// dataSource->setUpdateInterval(1000); // Not relevant
	dataSource->setSourceType(LiveDataSource::SourceType::FileOrPipe);
	dataSource->setReadingType(LiveDataSource::ReadingType::WholeFile);
	dataSource->setFileName(QStringLiteral(EXPORT_FILE));
	dataSource->setUpdateType(LiveDataSource::UpdateType::NewData);

	qint64 lastUpdate = 0;
	int counter = 0;
	connect(dataSource, &LiveDataSource::readOnUpdateCalled, [&lastUpdate, &counter](bool paused) {
		const qint64 t = QDateTime::currentDateTime().toMSecsSinceEpoch();
		counter++;

		QCOMPARE(paused, false);

		if (lastUpdate > 0) {
			QVERIFY(abs(lastUpdate - t - SLEEP_TIME_MS) < static_cast<double>(SLEEP_TIME_MS) / 10); // maximum 10% of delay
			lastUpdate = t;
		}
	});

	qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
	bool timeout = false;
	while (counter < 100 && !timeout) {
		const qint64 currTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
		timeout = (currTime - startTime) > (100 * SLEEP_TIME_MS) * 1.1;
		QApplication::processEvents();
	}

	process.terminate();
	QVERIFY(process.waitForFinished());
	QCOMPARE(timeout, false);
}

QTEST_MAIN(LiveDataWriteFileTest)
