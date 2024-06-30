/*
	File                 : ROOTFilterTest.cpp
	Project              : LabPlot
	Description          : Tests for the ROOT filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ROOTFilterTest.h"
#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

void ROOTFilterTest::initTestCase() {
	KLocalizedString::setApplicationDomain("labplot2");
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	// TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

void ROOTFilterTest::importFile1() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/advanced_zlib.root"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ROOTFilter filter;
	filter.setStartRow(1);
	// filter.setEndRow(100);
	filter.setCurrentObject(QStringLiteral("Hist:variableBinHist;2"));
	QVector<QStringList> columns{{QStringLiteral("center")}, {QStringLiteral("content")}, {QStringLiteral("error")}};
	filter.setColumns(columns);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 101);

	//	WARN(spreadsheet.column(0)->valueAt(0))
	QCOMPARE(spreadsheet.column(0)->valueAt(0), -4.95495);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), -4.86475);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(99), 4.94505);
	QCOMPARE(spreadsheet.column(1)->valueAt(99), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(99), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(100), INFINITY);
	QCOMPARE(spreadsheet.column(1)->valueAt(100), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(100), 0);

	ROOTFilter filter2;
	filter2.setStartRow(0);
	filter2.setEndRow(9); // TODO: automatic?
	filter2.setCurrentObject(QStringLiteral("Tree:tree"));
	QVector<QStringList> columns2{{QStringLiteral("doubleTest")}, {QStringLiteral("structTest"), QStringLiteral("double")}};
	filter2.setColumns(columns2);
	filter2.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 2);
	QCOMPARE(spreadsheet.rowCount(), 10);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 81);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), 1);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 64);
	QCOMPARE(spreadsheet.column(0)->valueAt(8), 8);
	QCOMPARE(spreadsheet.column(1)->valueAt(8), 1);
	QCOMPARE(spreadsheet.column(0)->valueAt(9), 9);
	QCOMPARE(spreadsheet.column(1)->valueAt(9), 0);
}

void ROOTFilterTest::importFile2() {
	const QString& fileName = QFINDTESTDATA(QLatin1String("data/basic_lz4.root"));

	Spreadsheet spreadsheet(QStringLiteral("test"), false);
	ROOTFilter filter;
	filter.setStartRow(1);
	// filter.setEndRow(100);
	filter.setCurrentObject(QStringLiteral("Hist:doubleHist;1"));
	QVector<QStringList> columns{{QStringLiteral("center")}, {QStringLiteral("content")}, {QStringLiteral("error")}};
	filter.setColumns(columns);
	filter.readDataFromFile(fileName, &spreadsheet);

	QCOMPARE(spreadsheet.columnCount(), 3);
	QCOMPARE(spreadsheet.rowCount(), 101);

	QCOMPARE(spreadsheet.column(0)->valueAt(0), -4.95);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(1)->valueAt(0), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(1), -4.85);
	QCOMPARE(spreadsheet.column(1)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(1), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(99), 4.95);
	QCOMPARE(spreadsheet.column(1)->valueAt(99), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(99), 0);
	QCOMPARE(spreadsheet.column(0)->valueAt(100), INFINITY);
	QCOMPARE(spreadsheet.column(1)->valueAt(100), 0);
	QCOMPARE(spreadsheet.column(2)->valueAt(100), 0);
}

// BENCHMARKS

/*
void ROOTFilterTest::benchDoubleImport_data() {
	QTest::addColumn<long>("lineCount");
	// can't transfer file name since needed in clean up

	QTemporaryFile file;
	if (!file.open()) // needed to generate file name
		return;
	file.close(); // only file name is used

	benchDataFileName = file.fileName();
	benchDataFileName.append(".fits");

	QString testName(QString::number(paths) + QLatin1String(" random double paths"));

	QTest::newRow(testName.toLatin1()) << lines;
	DEBUG("CREATE DATA FILE " << STDSTRING(benchDataFileName) << ", lines = " << lines)

	gsl_rng_env_setup();
	gsl_rng* r = gsl_rng_alloc(gsl_rng_default);
	gsl_rng_set(r, 12345);

	// create file

	// create data
	double path[paths] = {0.0};
	double* data = new double[paths * lines];

	const double delta = 0.25;
	const int dt = 1;
	const double sigma = delta * delta * dt;
	for (long i = 0; i < lines; ++i) {
		// std::cout << "line " << i+1 << std::endl;

		for (int p = 0; p < paths; ++p) {
			path[p] += gsl_ran_gaussian_ziggurat(r, sigma);
			data[p + i * paths] = path[p];
		}
	}

	// write data and clean up

	delete[] data;

	DEBUG(Q_FUNC_INFO << ", DONE")
}

void ROOTFilterTest::benchDoubleImport() {
	Spreadsheet spreadsheet("test", false);
	ROOTFilter filter;

	const int p = paths; // need local variable
	QBENCHMARK {
		filter.readDataFromFile(benchDataFileName, &spreadsheet);

		QCOMPARE(spreadsheet.columnCount(), p);
		QCOMPARE(spreadsheet.rowCount(), lines);

		QCOMPARE(spreadsheet.column(0)->valueAt(0), 0.120997813055);
		QCOMPARE(spreadsheet.column(1)->valueAt(0), 0.119301077563219);
		QCOMPARE(spreadsheet.column(2)->valueAt(0), -0.0209979608555485);
	}
}

void ROOTFilterTest::benchDoubleImport_cleanup() {
	DEBUG("REMOVE DATA FILE " << STDSTRING(benchDataFileName))
	QFile::remove(benchDataFileName);
}
*/

QTEST_MAIN(ROOTFilterTest)
