/*
	File                 : BinaryFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the binary filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BinaryFilterTest.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

void BinaryFilterTest::importInt8() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int8.bin"));
	filter.setDataType(BinaryFilter::DataType::INT8);
	// filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50);
}

void BinaryFilterTest::importInt16BE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int16.bin"));
	filter.setDataType(BinaryFilter::DataType::INT16);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50);
}

void BinaryFilterTest::importInt32LE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int32LE.bin"));
	filter.setDataType(BinaryFilter::DataType::INT32);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 700);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 2);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 3);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 2);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 4);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 3);
	QCOMPARE(spreadsheet.column(0)->valueAt(698), 698);
	QCOMPARE(spreadsheet.column(1)->valueAt(698), 64);
	QCOMPARE(spreadsheet.column(0)->valueAt(699), 699);
	QCOMPARE(spreadsheet.column(1)->valueAt(699), 64);
}

void BinaryFilterTest::importInt32BE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int32.bin"));
	filter.setDataType(BinaryFilter::DataType::INT32);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Integer);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Integer);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50);
}

void BinaryFilterTest::importInt64BE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/int64.bin"));
	filter.setDataType(BinaryFilter::DataType::INT64);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::BigInt);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::BigInt);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50);
}

void BinaryFilterTest::importFloatBE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/float.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL32);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	// DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(0)->valueAt(999))
	// DEBUG(Q_FUNC_INFO << ", value = " << spreadsheet.column(1)->valueAt(999))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.100100100040436);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9.9932918548584);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0.200200200080872);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19.8865337371826);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.300300300121307);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29.5806789398193);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0.400400400161743);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38.9786720275879);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99.89990234375);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59.0004920959473);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50.6365623474121);
}

void BinaryFilterTest::importDoubleBE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/double.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL64);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 1000);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.1001001001001);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 9.9932916564023);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0.2002002002002);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 19.8865340135936);
	QCOMPARE(spreadsheet.column(0)->valueAt(3), 0.3003003003003);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 29.5806794305369);
	QCOMPARE(spreadsheet.column(0)->valueAt(4), 0.4004004004004);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 38.978673554296);
	QCOMPARE(spreadsheet.column(0)->valueAt(998), 99.8998998998999);
	QCOMPARE(spreadsheet.column(1)->valueAt(998), -59.000490423342);
	QCOMPARE(spreadsheet.column(0)->valueAt(999), 100);
	QCOMPARE(spreadsheet.column(1)->valueAt(999), -50.6365641109759);
}

void BinaryFilterTest::importDoubleLE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/doubleLE.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL64);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 700);

	QCOMPARE(spreadsheet.column(0)->columnMode(), AbstractColumn::ColumnMode::Double);
	QCOMPARE(spreadsheet.column(1)->columnMode(), AbstractColumn::ColumnMode::Double);

	for (int i = 0; i < 700; i++)
		QCOMPARE(spreadsheet.column(0)->valueAt(i), i * 0.01);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.999983333416666);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 1.99986666933331);
	QCOMPARE(spreadsheet.column(1)->valueAt(3), 2.99955002024957);
	QCOMPARE(spreadsheet.column(1)->valueAt(4), 3.99893341866342);
	QCOMPARE(spreadsheet.column(1)->valueAt(698), 64.1778165874934);
	QCOMPARE(spreadsheet.column(1)->valueAt(699), 64.9414852768911);
}

void BinaryFilterTest::importDoubleMatrixBE() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/matrix_double.bin"));
	filter.setDataType(BinaryFilter::DataType::REAL64);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.setVectors(40);
	filter.readDataFromFile(fileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

	// spreadsheet size
	QCOMPARE(spreadsheet.columnCount(), 40);
	QCOMPARE(spreadsheet.rowCount(), 40);

	for (int i = 0; i < 40; i++)
		QCOMPARE(spreadsheet.column(i)->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.0256381904333);
	QCOMPARE(spreadsheet.column(2)->valueAt(0), 0.0512595256828);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 0.0256381904333);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0.0512595256828);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0.0768471616458);
	QCOMPARE(spreadsheet.column(0)->valueAt(2), 0.0512595256828);
	QCOMPARE(spreadsheet.column(1)->valueAt(2), 0.0768471616458);
	QCOMPARE(spreadsheet.column(2)->valueAt(2), 0.102384276374254);
	QCOMPARE(spreadsheet.column(38)->valueAt(38), 0.929433523316);
	QCOMPARE(spreadsheet.column(38)->valueAt(39), 0.919667781302);
	QCOMPARE(spreadsheet.column(39)->valueAt(38), 0.91966778130227);
	QCOMPARE(spreadsheet.column(39)->valueAt(39), 0.909297426825682);
}

/////////////////////////////////////////////////////////////////

// INT data

void BinaryFilterTest::benchIntImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	// create data
	QTemporaryFile file;
	if (!file.open())
		return;

	file.setAutoRemove(false);
	benchDataFileName = file.fileName();

	QDataStream out(&file);

	QTest::newRow("3 int cols");
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)
	for (size_t i = 0; i < lines; i++)
		out << static_cast<int>(i) << static_cast<int>(100 * sin(i / 100.)) << static_cast<int>(100 * cos(i / 100));
	file.close();
}

void BinaryFilterTest::benchIntImport() {
	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	filter.setDataType(BinaryFilter::DataType::INT32);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.setVectors(3);

	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), 3);
		QCOMPARE(spreadsheet.rowCount(), lines);

		QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
		QCOMPARE(spreadsheet.column(2)->valueAt(0), 100);
	}
}

void BinaryFilterTest::benchIntImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

// DOUBLE data

void BinaryFilterTest::benchDoubleImport_data() {
	QTest::addColumn<size_t>("lineCount");
	// can't transfer file name since needed in clean up

	// create data
	QTemporaryFile file;
	if (!file.open())
		return;

	file.setAutoRemove(false);
	benchDataFileName = file.fileName();

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	QDataStream out(&file);

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));
	QTest::newRow(qPrintable(testName)) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta * delta * dt;
	double path[paths] = {0.0};
	for (size_t i = 0; i < lines; i++) {
		for (int p = 0; p < paths; p++) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			out << path[p];
		}
	}
	file.close();
}

void BinaryFilterTest::benchDoubleImport() {
	QFETCH(size_t, lineCount);

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	BinaryFilter filter;
	filter.setDataType(BinaryFilter::DataType::REAL64);
	filter.setByteOrder(QDataStream::ByteOrder::BigEndian);
	filter.setVectors(paths);

	const int p = paths; // need local variable
	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet, AbstractFileFilter::ImportMode::Replace);

		QCOMPARE(spreadsheet.columnCount(), p);
		QCOMPARE(spreadsheet.rowCount(), lineCount);

		QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.120997813055);
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.119301077563219);
		QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.0209979608555485);
	}
}

void BinaryFilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}

///////////////////////////////////////////////////////

QTEST_MAIN(BinaryFilterTest)
